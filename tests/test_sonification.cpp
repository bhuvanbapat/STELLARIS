#include "test_harness.h"

#include "audio/AudioEngine.h"
#include "sim/Sonification.h"

#include <cmath>
#include <vector>

using namespace stellaris;

TEST(asteroseismic_scaling_reproduces_the_sun) {
    CHECK_NEAR(numaxMicroHz(1.0, 1.0, kSolarTeffK), kSolarNuMaxMicroHz, 1e-9);
    CHECK_NEAR(deltaNuMicroHz(1.0, 1.0), kSolarDeltaNuMicroHz, 1e-9);
    CHECK_NEAR(audiblePModeCenterHz(kSolarNuMaxMicroHz), 220.0, 1e-9);
    // A red giant (1 Msun, 10 Rsun, 4800 K) oscillates ~100x slower: nu_max ~ 33 muHz.
    const double giant = numaxMicroHz(1.0, 10.0, 4800.0);
    CHECK(giant > 25.0 && giant < 40.0);
    CHECK(audiblePModeCenterHz(giant) < 220.0 && audiblePModeCenterHz(giant) > 40.0);
}

TEST(sonification_follows_the_stages) {
    const ModelConfig sun{1.0, 1.0};
    const SonificationParams cloud = sonify(evaluateScene(8.0, sun));
    CHECK(cloud.cloudLevel > 0.9f && cloud.pmodeLevel == 0.0f && cloud.jetLevel == 0.0f);
    const SonificationParams jets = sonify(evaluateScene(72.0, sun));
    CHECK(jets.jetLevel > 0.9f);
    const SonificationParams ms = sonify(evaluateScene(94.0, sun));
    CHECK(ms.pmodeLevel > 0.5f && std::fabs(ms.pmodeCenterHz - 220.0f) < 25.0f);
    // The giant oscillates at a lower pitch than the main-sequence star.
    const SonificationParams giant = sonify(evaluateScene(106.0, sun));
    CHECK(giant.pmodeLevel > 0.3f && giant.pmodeCenterHz < ms.pmodeCenterHz);
    // Endings: nebula chord + white-dwarf tone; pulsar; black hole.
    const SonificationParams pn = sonify(evaluateScene(140.0, sun));
    CHECK(pn.nebulaLevel > 0.5f && pn.remnantToneLevel > 0.5f && pn.boomLevel == 0.0f);
    const SonificationParams ns = sonify(evaluateScene(147.0, ModelConfig{15.0, 1.0}));
    CHECK(ns.pulsarLevel > 0.5f && std::fabs(ns.pulsarHz - 29.9f) < 0.01f && ns.blackHoleLevel == 0.0f);
    const SonificationParams bh = sonify(evaluateScene(147.0, ModelConfig{40.0, 1.0}));
    CHECK(bh.blackHoleLevel > 0.5f && bh.pulsarLevel == 0.0f);
    // Deterministic: same input, same parameters.
    const SonificationParams again = sonify(evaluateScene(94.0, sun));
    CHECK(again.pmodeCenterHz == ms.pmodeCenterHz && again.pmodeSpacingHz == ms.pmodeSpacingHz);
}

namespace {
float peakOf(AudioEngine& engine, double seconds) {
    std::vector<float> buf(2 * 4800);
    float peak = 0.0f;
    for (int i = 0; i < static_cast<int>(seconds * 10.0); ++i) {
        engine.render(buf.data(), 4800);
        for (float v : buf) {
            if (!std::isfinite(v)) return 1e9f;
            peak = std::max(peak, std::fabs(v));
        }
    }
    return peak;
}
} // namespace

TEST(audio_output_is_bounded_and_mute_is_silent) {
    AudioEngine engine;  // no device: render() is driven directly
    engine.setVolume(1.0f);
    // Worst case: every voice at once, including the supernova, at full volume.
    SonificationParams loud = sonify(evaluateScene(72.0, ModelConfig{1.0, 1.0}));
    loud.cloudLevel = loud.burstLevel = loud.jetLevel = loud.pmodeLevel = loud.nebulaLevel = 1.0f;
    loud.boomLevel = loud.pulsarLevel = loud.blackHoleLevel = loud.remnantToneLevel = 1.0f;
    loud.pmodeCenterHz = 220.0f; loud.pmodeSpacingHz = 9.6f; loud.nebulaBaseHz = 196.0f;
    loud.remnantToneHz = 660.0f; loud.pulsarHz = 29.9f;
    engine.publish(loud, true);
    const float peak = peakOf(engine, 3.0);
    CHECK(peak > 0.01f && peak <= 0.851f);  // audible, finite, under the soft-clip ceiling
    engine.setMuted(true);
    peakOf(engine, 0.5);                       // let the 80 ms mute ramp finish
    CHECK(peakOf(engine, 0.5) < 1e-4f);
}
