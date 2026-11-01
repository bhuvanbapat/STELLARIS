#pragma once
// Simplified, documented stellar relations used by STELLARIS.
// Every function here is a rough textbook-level scaling relation, NOT a stellar
// evolution model. See docs/SCIENCE_NOTES.md for sources and limitations.

#include <glm/vec3.hpp>

namespace stellaris {

// Range of the user-selectable initial mass, in solar masses (M_sun).
// 1 M_sun is the lower limit because lower-mass stars have main-sequence
// lifetimes comparable to or longer than the current age of the Universe, so
// none of them has yet completed the final stage being shown.
inline constexpr double kMinMassSolar = 1.0;
inline constexpr double kMaxMassSolar = 40.0;
inline constexpr double kDefaultMassSolar = 1.0;

// Illustrative category boundaries (M_sun). These are conventional, approximate
// values: the real boundaries depend on metallicity, rotation, mass loss and
// binary interaction, and the NS/BH outcome is not even monotonic in mass.
inline constexpr double kWhiteDwarfUpperMass  = 8.0;   // literature range roughly 6-12
inline constexpr double kNeutronStarUpperMass = 25.0;  // highly uncertain, ~20-30
// Masses inside these bands are flagged in the UI as "boundary uncertain".
inline constexpr double kWhiteDwarfBandLow  = 6.0,  kWhiteDwarfBandHigh  = 10.0;
inline constexpr double kNeutronStarBandLow = 18.0, kNeutronStarBandHigh = 30.0;

// Nominal solar effective temperature, IAU 2015 Resolution B3 (K).
inline constexpr double kSolarTeffK = 5772.0;

enum class Pathway { WhiteDwarf, NeutronStar, BlackHole };

const char* pathwayName(Pathway p);        // e.g. "Low / intermediate mass"
const char* remnantName(Pathway p);        // e.g. "White dwarf"
Pathway pathwayForMass(double massSolar);  // illustrative category
bool massNearCategoryBoundary(double massSolar);

struct MainSequenceProperties {
    double massSolar;        // parameter
    double luminositySolar;  // calculated: piecewise mass-luminosity relation
    double radiusSolar;      // calculated: two-piece empirical mass-radius fit
    double teffK;            // calculated: Stefan-Boltzmann, scaled to solar values
    double lifetimeYears;    // calculated: t ~ 1e10 yr * (M/L), order of magnitude only
};

double luminosityFromMass(double massSolar);
double radiusFromMass(double massSolar);
double effectiveTemperature(double luminositySolar, double radiusSolar);
double mainSequenceLifetimeYears(double massSolar, double luminositySolar);
MainSequenceProperties mainSequenceProperties(double massSolar);

// Approximate linear-sRGB color of a blackbody at temperature T (K), normalized
// so the largest channel is 1. Computed by integrating Planck's law against an
// analytic fit of the CIE 1931 color-matching functions.
glm::vec3 blackbodyColor(double temperatureK);

// Free-fall time t_ff = sqrt(3*pi / (32 G rho)) for a uniform, pressureless
// sphere of molecular hydrogen number density n(H2) [cm^-3], mean mass per
// particle 2.8 m_H (includes helium). Returns years.
double freeFallTimeYears(double numberDensityH2PerCm3);

} // namespace stellaris
