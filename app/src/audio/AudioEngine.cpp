#include "audio/AudioEngine.h"

#include "core/Log.h"

#include <algorithm>
#include <cmath>
#include <cstdint>

#define NOMINMAX
#define MA_NO_DECODING
#define MA_NO_ENCODING
#define MA_NO_RESOURCE_MANAGER
#define MA_NO_NODE_GRAPH
#define MA_NO_ENGINE
#define MA_NO_GENERATION
#define MINIAUDIO_IMPLEMENTATION  // the single translation unit that compiles miniaudio
#include <miniaudio.h>

namespace stellaris {

namespace {

constexpr float kTwoPi = 6.28318530718f;
constexpr float kFs = static_cast<float>(AudioEngine::kSampleRate);

enum Param {
    Ambient, CloudLevel, CloudPitch, Burst, Jet, KnotRate, PLevel, PCenter, PSpacing,
    NebLevel, NebBase, Boom, RemLevel, RemHz, PulsarLevel, PulsarHz, BhLevel, Playing, kCount
};

// One-pole smoothing coefficient for a time constant (seconds) at the sample rate.
float coef(float seconds) { return 1.0f - std::exp(-1.0f / (seconds * kFs)); }

struct Rng {
    uint32_t s;
    float next() {  // uniform in [-1, 1)
        s ^= s << 13; s ^= s >> 17; s ^= s << 5;
        return static_cast<float>(s) * (2.0f / 4294967296.0f) - 1.0f;
    }
};

// Topology-preserving state-variable filter (Zavalishin); band-pass and low-pass outputs.
struct Svf {
    float ic1 = 0.0f, ic2 = 0.0f;
    float bp = 0.0f, lp = 0.0f;
    void process(float x, float hz, float q) {
        const float g = std::tan(3.14159265f * std::clamp(hz, 10.0f, 0.45f * kFs) / kFs);
        const float k = 1.0f / q;
        const float a1 = 1.0f / (1.0f + g * (g + k)), a2 = g * a1, a3 = g * a2;
        const float v3 = x - ic2;
        const float v1 = a1 * ic1 + a2 * v3;
        const float v2 = ic2 + a2 * ic1 + a3 * v3;
        ic1 = 2.0f * v1 - ic1;
        ic2 = 2.0f * v2 - ic2;
        bp = v1;
        lp = v2;
    }
};

// Small Schroeder reverb (4 parallel combs + 2 series all-passes) per channel.
struct Reverb {
    static constexpr int kComb[4] = {1557, 1617, 1491, 1422};
    static constexpr int kAll[2] = {556, 441};
    float comb[4][1700] = {};
    float all[2][600] = {};
    int ci[4] = {}, ai[2] = {};
    float damp[4] = {};
    float process(float x, int spread) {
        float out = 0.0f;
        for (int i = 0; i < 4; ++i) {
            const int n = kComb[i] + spread;
            float& cell = comb[i][ci[i]];
            const float y = cell;
            damp[i] = y * 0.6f + damp[i] * 0.4f;
            cell = x + damp[i] * 0.80f;
            ci[i] = (ci[i] + 1) % n;
            out += y;
        }
        out *= 0.25f;
        for (int i = 0; i < 2; ++i) {
            const int n = kAll[i] + spread / 2;
            float& cell = all[i][ai[i]];
            const float buf = cell;
            cell = out + buf * 0.5f;
            out = buf - out;
            ai[i] = (ai[i] + 1) % n;
        }
        return out;
    }
};

constexpr int kModes = 22;  // l = 0 and l = 1 ridges, 11 orders each

} // namespace

struct AudioEngine::Synth {
    float cur[kCount] = {};
    Rng rng{0x9E3779B9u};
    Rng rngR{0x7F4A7C15u};
    // Phases.
    float modePhase[kModes] = {};
    float modeAmp[kModes] = {}, modeTarget[kModes] = {};
    int modeTimer = 0;
    float nebPhase[4] = {}, remPhase = 0.0f, bhPhase[3] = {}, boomPhase = 0.0f, knotPhase = 0.0f;
    float pulsarPhase = 0.0f, pulseEnv = 0.0f, pulseAmp = 1.0f, slowPhase = 0.0f;
    float brownL = 0.0f, brownR = 0.0f;
    float boomEnv = 0.0f, boomAge = 0.0f;
    float gain = 0.0f;
    float peak = 0.0f;
    Svf cloudL, cloudR, jetL, jetR, burstF, boomF, bhF, pulseF;
    Reverb revL, revR;
};

struct AudioEngine::Device {
    ma_device device{};
    bool initialized = false;
};

namespace {
void dataCallback(ma_device* device, void* output, const void*, ma_uint32 frames) {
    static_cast<AudioEngine*>(device->pUserData)->render(static_cast<float*>(output), frames);
}
} // namespace

AudioEngine::AudioEngine() : synth_(std::make_unique<Synth>()), device_(std::make_unique<Device>()) {
    for (auto& t : target_) t.store(0.0f);
    target_[Playing].store(1.0f);
}

AudioEngine::~AudioEngine() { stop(); }

bool AudioEngine::start() {
    if (running_) return true;
    ma_device_config cfg = ma_device_config_init(ma_device_type_playback);
    cfg.playback.format = ma_format_f32;
    cfg.playback.channels = 2;
    cfg.sampleRate = kSampleRate;
    cfg.dataCallback = dataCallback;
    cfg.pUserData = this;
    if (ma_device_init(nullptr, &cfg, &device_->device) != MA_SUCCESS) {
        log::warn("Audio: no playback device available; sonification disabled");
        return false;
    }
    device_->initialized = true;
    if (ma_device_start(&device_->device) != MA_SUCCESS) {
        log::warn("Audio: could not start the playback device; sonification disabled");
        ma_device_uninit(&device_->device);
        device_->initialized = false;
        return false;
    }
    running_ = true;
    log::info("Audio: {} Hz stereo on \"{}\"", kSampleRate, device_->device.playback.name);
    return true;
}

void AudioEngine::stop() {
    if (device_ && device_->initialized) {
        ma_device_uninit(&device_->device);  // stops the callback thread first
        device_->initialized = false;
    }
    running_ = false;
}

void AudioEngine::publish(const SonificationParams& p, bool playing) {
    const float v[kCount] = {p.ambient, p.cloudLevel, p.cloudPitchHz, p.burstLevel, p.jetLevel, p.knotRateHz,
                             p.pmodeLevel, p.pmodeCenterHz, p.pmodeSpacingHz, p.nebulaLevel, p.nebulaBaseHz,
                             p.boomLevel, p.remnantToneLevel, p.remnantToneHz, p.pulsarLevel, p.pulsarHz,
                             p.blackHoleLevel, playing ? 1.0f : 0.0f};
    for (int i = 0; i < kCount; ++i) target_[i].store(v[i], std::memory_order_relaxed);
}

void AudioEngine::render(float* out, unsigned frames) {
    Synth& s = *synth_;
    float tgt[kCount];
    for (int i = 0; i < kCount; ++i) tgt[i] = target_[i].load(std::memory_order_relaxed);
    // Pitch targets snap when their voice is silent (no audible glide from zero).
    for (int pitch : {CloudPitch, PCenter, PSpacing, NebBase, RemHz, PulsarHz, KnotRate})
        if (s.cur[pitch] <= 0.0f) s.cur[pitch] = tgt[pitch];
    const float masterTarget = (muted_.load() ? 0.0f : std::clamp(volume_.load(), 0.0f, 1.0f));

    const float kLevel = coef(0.35f), kPitch = coef(0.6f), kGain = coef(0.04f);
    const float kBoomUp = coef(0.25f), kBoomDown = coef(2.5f);
    // Line chord: H-alpha, [N II] 658.4 (beats with H-alpha), [O III] 500.7, He II 468.6.
    const float lineRatio[4] = {1.0f, 656.3f / 658.4f, 656.3f / 500.7f, 656.3f / 468.6f};
    const float lineAmp[4] = {0.8f, 0.45f, 1.0f, 0.3f};

    for (unsigned f = 0; f < frames; ++f) {
        for (int i = 0; i < kCount; ++i) {
            const bool pitch = i == CloudPitch || i == PCenter || i == PSpacing || i == NebBase || i == RemHz ||
                               i == PulsarHz || i == KnotRate;
            if (i == Boom) continue;
            s.cur[i] += (tgt[i] - s.cur[i]) * (pitch ? kPitch : kLevel);
        }
        // Supernova envelope: 0.25 s attack, 2.5 s release; the shock rumble darkens with age.
        s.boomEnv += (tgt[Boom] - s.boomEnv) * (tgt[Boom] > s.boomEnv ? kBoomUp : kBoomDown);
        s.boomAge = s.boomEnv > 0.01f ? s.boomAge + 1.0f / kFs : 0.0f;

        const float nL = s.rng.next(), nR = s.rngR.next();
        float L = 0.0f, R = 0.0f;

        // Ambient bed: very quiet brown noise.
        s.brownL = s.brownL * 0.997f + nL * 0.02f;
        s.brownR = s.brownR * 0.997f + nR * 0.02f;
        L += s.brownL * 0.12f * s.cur[Ambient];
        R += s.brownR * 0.12f * s.cur[Ambient];

        // Cloud: decorrelated band-passed noise per channel (wide, airy).
        if (s.cur[CloudLevel] > 1e-4f) {
            s.cloudL.process(nL, s.cur[CloudPitch], 1.4f);
            s.cloudR.process(nR, s.cur[CloudPitch] * 1.07f, 1.4f);
            L += s.cloudL.bp * 0.30f * s.cur[CloudLevel];
            R += s.cloudR.bp * 0.30f * s.cur[CloudLevel];
        }

        // Accretion burst: a low swell.
        if (s.cur[Burst] > 1e-4f) {
            s.burstF.process((nL + nR) * 0.5f, 85.0f, 2.0f);
            const float b = s.burstF.bp * 0.9f * s.cur[Burst];
            L += b; R += b;
        }

        // Jets: bright hiss pulsing as knots pass.
        if (s.cur[Jet] > 1e-4f) {
            s.knotPhase += s.cur[KnotRate] / kFs;
            s.knotPhase -= std::floor(s.knotPhase);
            const float knot = 0.55f + 0.45f * std::pow(0.5f + 0.5f * std::sin(kTwoPi * s.knotPhase), 3.0f);
            s.jetL.process(nL, 3200.0f, 0.9f);
            s.jetR.process(nR, 3600.0f, 0.9f);
            L += s.jetL.bp * 0.10f * s.cur[Jet] * knot;
            R += s.jetR.bp * 0.10f * s.cur[Jet] * knot;
        }

        // Stellar oscillations: stochastically excited modes on the asymptotic pattern
        // nu = nu_max + Delta_nu (k + l/2), amplitudes in a Gaussian envelope.
        if (s.cur[PLevel] > 1e-4f) {
            if (--s.modeTimer <= 0) {
                s.modeTimer = static_cast<int>(kFs * 0.35f);
                for (int m = 0; m < kModes; ++m) s.modeTarget[m] = 0.5f + 0.5f * s.rng.next();
            }
            float sumL = 0.0f, sumR = 0.0f;
            for (int m = 0; m < kModes; ++m) {
                const int l = m & 1;
                const float k = static_cast<float>(m / 2) - 5.0f;
                const float off = k + 0.5f * static_cast<float>(l);
                const float hz = s.cur[PCenter] + s.cur[PSpacing] * off;
                s.modeAmp[m] += (s.modeTarget[m] - s.modeAmp[m]) * 0.00002f;
                s.modePhase[m] += hz / kFs;
                s.modePhase[m] -= std::floor(s.modePhase[m]);
                const float env = std::exp(-off * off / (2.0f * 3.2f * 3.2f)) * (l ? 0.75f : 1.0f);
                const float v = std::sin(kTwoPi * s.modePhase[m]) * env * s.modeAmp[m];
                const float pan = 0.5f + 0.35f * std::sin(static_cast<float>(m) * 1.7f);
                sumL += v * (1.0f - pan);
                sumR += v * pan;
            }
            L += sumL * 0.045f * s.cur[PLevel];
            R += sumR * 0.045f * s.cur[PLevel];
        }

        // Planetary nebula: emission-line chord with slow tremolo.
        if (s.cur[NebLevel] > 1e-4f) {
            s.slowPhase += 0.11f / kFs;
            s.slowPhase -= std::floor(s.slowPhase);
            float sum = 0.0f;
            for (int i = 0; i < 4; ++i) {
                s.nebPhase[i] += s.cur[NebBase] * lineRatio[i] / kFs;
                s.nebPhase[i] -= std::floor(s.nebPhase[i]);
                sum += std::sin(kTwoPi * s.nebPhase[i]) * lineAmp[i];
            }
            const float trem = 0.8f + 0.2f * std::sin(kTwoPi * s.slowPhase);
            L += sum * 0.040f * s.cur[NebLevel] * trem;
            R += sum * 0.040f * s.cur[NebLevel] * (1.8f - trem);
        }

        // Supernova: low rumble whose filter closes as the shock ages, plus a falling sub tone.
        if (s.boomEnv > 1e-4f) {
            const float cutoff = 90.0f + 700.0f * std::exp(-s.boomAge / 1.2f);
            s.boomF.process((nL + nR) * 0.5f, cutoff, 0.8f);
            const float subHz = 32.0f + 40.0f * std::exp(-s.boomAge / 1.5f);
            s.boomPhase += subHz / kFs;
            s.boomPhase -= std::floor(s.boomPhase);
            const float b = (s.boomF.lp * 1.6f + std::sin(kTwoPi * s.boomPhase) * 0.35f) * s.boomEnv;
            L += b; R += b;
        }

        // White dwarf: faint high tone, pitch from temperature.
        if (s.cur[RemLevel] > 1e-4f) {
            s.remPhase += s.cur[RemHz] / kFs;
            s.remPhase -= std::floor(s.remPhase);
            const float v = (std::sin(kTwoPi * s.remPhase) + 0.2f * std::sin(2.0f * kTwoPi * s.remPhase)) * 0.025f *
                            s.cur[RemLevel];
            L += v; R += v;
        }

        // Pulsar: one short, slightly variable click per rotation (a buzz at ~30 Hz).
        if (s.cur[PulsarLevel] > 1e-4f) {
            s.pulsarPhase += s.cur[PulsarHz] / kFs;
            if (s.pulsarPhase >= 1.0f) {
                s.pulsarPhase -= 1.0f;
                s.pulseEnv = 1.0f;
                s.pulseAmp = 0.7f + 0.3f * s.rng.next();  // pulse-to-pulse variability
            }
            s.pulseEnv *= 0.9965f;
            s.pulseF.process(nL * s.pulseEnv, 1400.0f, 1.2f);
            const float v = s.pulseF.bp * 0.9f * s.pulseAmp * s.cur[PulsarLevel];
            L += v; R += v;
        }

        // Black hole: a very low, slowly beating drone with dark noise.
        if (s.cur[BhLevel] > 1e-4f) {
            const float hz[3] = {36.0f, 36.6f, 54.0f};
            float sum = 0.0f;
            for (int i = 0; i < 3; ++i) {
                s.bhPhase[i] += hz[i] / kFs;
                s.bhPhase[i] -= std::floor(s.bhPhase[i]);
                sum += std::sin(kTwoPi * s.bhPhase[i]) * (i == 2 ? 0.4f : 1.0f);
            }
            s.bhF.process((nL + nR) * 0.5f, 70.0f, 0.7f);
            const float v = (sum * 0.10f + s.bhF.lp * 0.5f) * s.cur[BhLevel];
            L += v; R += v;
        }

        // Space: shared reverb for depth.
        const float wetL = s.revL.process(L, 0), wetR = s.revR.process(R, 23);
        L = L * 0.8f + wetL * 0.35f;
        R = R * 0.8f + wetR * 0.35f;

        // Master: smoothed gain (mute/volume/pause duck), peak follower and soft clip.
        const float duck = 0.55f + 0.45f * s.cur[Playing];
        s.gain += (masterTarget * duck - s.gain) * kGain;
        if (masterTarget == 0.0f && s.gain < 1e-3f) s.gain = 0.0f;  // muted means silent, not just quiet
        L *= s.gain;
        R *= s.gain;
        const float pk = std::max(std::fabs(L), std::fabs(R));
        s.peak = pk > s.peak ? s.peak + (pk - s.peak) * 0.01f : s.peak * 0.99995f;
        const float comp = s.peak > 0.5f ? 0.5f / s.peak : 1.0f;
        out[2 * f + 0] = std::tanh(L * comp * 1.1f) * 0.85f;
        out[2 * f + 1] = std::tanh(R * comp * 1.1f) * 0.85f;
    }
}

} // namespace stellaris
