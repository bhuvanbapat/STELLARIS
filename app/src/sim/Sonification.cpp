#include "sim/Sonification.h"

#include <algorithm>
#include <cmath>

namespace stellaris {

double numaxMicroHz(double massSolar, double radiusSolar, double teffK) {
    return kSolarNuMaxMicroHz * massSolar / (radiusSolar * radiusSolar) / std::sqrt(teffK / kSolarTeffK);
}

double deltaNuMicroHz(double massSolar, double radiusSolar) {
    return kSolarDeltaNuMicroHz * std::sqrt(massSolar / (radiusSolar * radiusSolar * radiusSolar));
}

double audiblePModeCenterHz(double numax) {
    return 220.0 * std::pow(std::max(numax, 1e-6) / kSolarNuMaxMicroHz, 0.15);
}

namespace {
float clamp01(float v) { return std::clamp(v, 0.0f, 1.0f); }
}

SonificationParams sonify(const SceneState& s) {
    SonificationParams p;
    p.ambient = 1.0f;

    // Cloud: present while the cloud exists; the band rises as the core contracts.
    p.cloudLevel = clamp01(s.cloudDensity) * (1.0f - 0.6f * s.giant);
    p.cloudPitchHz = 140.0f + 520.0f * s.collapse * s.collapse + 300.0f * s.coreGlow;

    p.burstLevel = clamp01(s.accretionBurst);
    p.jetLevel = clamp01(s.jetPresence);
    p.knotRateHz = 1.3f;  // same travelling-knot pattern as the jet shader (presentation rate)

    // Stellar oscillations: main-sequence star, then the expanding giant. The physical
    // radius of the giant is not in the visual model (its radius is compressed), so
    // a representative growth is assumed: ~150 R_sun (red giant / AGB) or ~700 R_sun
    // (red supergiant). Illustrative values, documented in SCIENCE_NOTES §9.7.
    const float starOn = clamp01(s.centralPresence) * clamp01(s.atmosphere + s.giant);
    if (starOn > 0.0f) {
        const double growth = s.pathway == Pathway::WhiteDwarf ? 150.0 : 700.0;
        const double radius = s.ms.radiusSolar * std::pow(growth, static_cast<double>(s.giant));
        const double numax = numaxMicroHz(s.ms.massSolar, radius, s.centralTempK);
        const double dnu = deltaNuMicroHz(s.ms.massSolar, radius);
        p.pmodeLevel = starOn;
        p.pmodeCenterHz = static_cast<float>(audiblePModeCenterHz(numax));
        p.pmodeSpacingHz = static_cast<float>(p.pmodeCenterHz * std::clamp(dnu / numax, 0.02, 0.25));
    }

    // Planetary nebula: emission-line chord, louder as the nebula is ionized.
    if (s.ejectaKind == 0) {
        p.nebulaLevel = clamp01(s.ejectaPresence) * (0.35f + 0.65f * clamp01(s.ejectaGlow));
        p.nebulaBaseHz = 196.0f;  // H-alpha (656.3 nm) mapped to G3
    } else {
        // Supernova: the engine smooths this envelope (no instantaneous spike).
        p.boomLevel = clamp01(s.supernovaFlash);
    }

    // White dwarf: a faint tone whose pitch rises with temperature (T^0.5).
    if (s.pathway == Pathway::WhiteDwarf) {
        p.remnantToneLevel = clamp01(s.remnantPresence);
        p.remnantToneHz = static_cast<float>(660.0 * std::sqrt(std::max(s.remnantTempK, 1000.0f) / 60000.0));
    }
    // Pulsar: the Crab pulsar spins at ~29.9 Hz; audible directly as a buzz of clicks.
    p.pulsarLevel = clamp01(s.pulsarBeams);
    p.pulsarHz = 29.9f;
    p.blackHoleLevel = clamp01(s.blackHole);
    return p;
}

} // namespace stellaris
