#pragma once
// Sonification: maps the evaluated scene to sound-synthesis parameters.
//
// Sound does not travel through space. Like NASA/Chandra and helioseismology
// sonifications, every sound here is a translation of a quantity into audio,
// chosen so the mapping is explainable (SCIENCE_NOTES.md §9.7):
//   - stars: solar-like p-mode oscillations from the asteroseismic scaling relations
//     (nu_max ~ M R^-2 Teff^-1/2, Delta_nu ~ sqrt(M/R^3)), shifted into hearing range;
//   - planetary nebula: a chord whose pitch ratios are the inverse wavelength ratios
//     of its strongest emission lines (H-alpha 656.3, [O III] 500.7, He II 468.6 nm);
//   - pulsar: the real Crab pulsar rotation rate (~30 Hz) as a click train, as in
//     radio-pulsar sonifications;
//   - black hole: a very low drone, after the Perseus-cluster pressure-wave sonification;
//   - cloud, collapse, jets, bursts and the supernova: noise bands whose level and
//     pitch follow the corresponding scene parameters (illustrative mapping).
// Pure function of the scene state, so it is deterministic and unit-testable.

#include "sim/SceneState.h"

namespace stellaris {

inline constexpr double kSolarNuMaxMicroHz   = 3090.0;  // Huber et al. 2011
inline constexpr double kSolarDeltaNuMicroHz = 135.1;   // Huber et al. 2011

// Asteroseismic scaling relations (Kjeldsen & Bedding 1995; Brown et al. 1991).
double numaxMicroHz(double massSolar, double radiusSolar, double teffK);
double deltaNuMicroHz(double massSolar, double radiusSolar);

// Audible centre frequency for a given nu_max: the solar value maps to 220 Hz and
// other stars follow (nu_max / nu_max_sun)^0.15. This logarithmic compression keeps
// giants (nu_max ~ 1/10^4 of the Sun's) audible while preserving the ordering.
double audiblePModeCenterHz(double numaxMicroHz);

struct SonificationParams {
    float ambient = 0.0f;        // very low cosmic bed, always present
    float cloudLevel = 0.0f;     // diffuse cloud: soft, wind-like noise band
    float cloudPitchHz = 0.0f;   // band centre, rises as the core contracts
    float burstLevel = 0.0f;     // accretion outburst swell
    float jetLevel = 0.0f;       // jet hiss
    float knotRateHz = 0.0f;     // amplitude pulses as knots pass
    float pmodeLevel = 0.0f;     // stellar oscillation comb
    float pmodeCenterHz = 0.0f;
    float pmodeSpacingHz = 0.0f; // large separation, mapped
    float nebulaLevel = 0.0f;
    float nebulaBaseHz = 0.0f;   // H-alpha line pitch; others follow wavelength ratios
    float boomLevel = 0.0f;      // supernova (smoothed in the engine, never a spike)
    float remnantToneLevel = 0.0f;  // white dwarf: faint high tone
    float remnantToneHz = 0.0f;     // follows the remnant temperature
    float pulsarLevel = 0.0f;
    float pulsarHz = 0.0f;
    float blackHoleLevel = 0.0f;
};

SonificationParams sonify(const SceneState& s);

} // namespace stellaris
