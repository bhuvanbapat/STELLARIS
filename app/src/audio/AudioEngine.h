#pragma once
// Real-time synthesis of the STELLARIS sonification (miniaudio device, 48 kHz stereo).
//
// The render thread publishes SonificationParams every frame; the audio thread reads
// them through atomics and smooths every parameter (no zipper noise, no jumps). Mute
// and volume ramp over ~40 ms, and a soft limiter bounds the output, so seeking into
// the supernova or toggling mute never produces a sudden spike. If no audio device is
// available the engine stays silent and the application runs normally.

#include "sim/Sonification.h"

#include <array>
#include <atomic>
#include <memory>

namespace stellaris {

class AudioEngine {
public:
    AudioEngine();
    ~AudioEngine();
    AudioEngine(const AudioEngine&) = delete;
    AudioEngine& operator=(const AudioEngine&) = delete;

    bool start();              // opens the default playback device; false if unavailable
    void stop();
    bool running() const { return running_; }

    void publish(const SonificationParams& p, bool playing);
    void setMuted(bool muted) { muted_.store(muted); }
    void setVolume(float v) { volume_.store(v); }

    // Renders interleaved stereo frames (used by the device callback and by tests).
    void render(float* out, unsigned frames);
    static constexpr unsigned kSampleRate = 48000;

private:
    struct Synth;
    std::unique_ptr<Synth> synth_;
    struct Device;
    std::unique_ptr<Device> device_;
    bool running_ = false;

    static constexpr int kParamCount = 18;
    std::array<std::atomic<float>, kParamCount> target_{};
    std::atomic<bool> muted_{false};
    std::atomic<float> volume_{0.7f};
};

} // namespace stellaris
