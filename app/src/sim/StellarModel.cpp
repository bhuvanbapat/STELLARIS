#include "sim/StellarModel.h"

#include <algorithm>
#include <cmath>

namespace stellaris {

const char* pathwayName(Pathway p) {
    switch (p) {
    case Pathway::WhiteDwarf:  return "Low / intermediate mass";
    case Pathway::NeutronStar: return "High mass";
    case Pathway::BlackHole:   return "Very high mass";
    }
    return "?";
}

const char* remnantName(Pathway p) {
    switch (p) {
    case Pathway::WhiteDwarf:  return "White dwarf";
    case Pathway::NeutronStar: return "Neutron star";
    case Pathway::BlackHole:   return "Black hole";
    }
    return "?";
}

Pathway pathwayForMass(double m) {
    if (m < kWhiteDwarfUpperMass) return Pathway::WhiteDwarf;
    if (m < kNeutronStarUpperMass) return Pathway::NeutronStar;
    return Pathway::BlackHole;
}

bool massNearCategoryBoundary(double m) {
    return (m >= kWhiteDwarfBandLow && m <= kWhiteDwarfBandHigh) ||
           (m >= kNeutronStarBandLow && m <= kNeutronStarBandHigh);
}

// Piecewise main-sequence mass-luminosity relation (Salaris & Cassisi 2005;
// Duric 2004), accurate only to within factors of order unity.
double luminosityFromMass(double m) {
    if (m < 0.43) return 0.23 * std::pow(m, 2.3);
    if (m < 2.0)  return std::pow(m, 4.0);
    if (m < 55.0) return 1.4 * std::pow(m, 3.5);
    return 32000.0 * m;
}

// Two-piece empirical main-sequence mass-radius fit attributed to
// Demircan & Kahraman (1991): R = 1.05 M^0.94 (M < 1.66), R = 1.37 M^0.54.
double radiusFromMass(double m) {
    if (m < 1.66) return 1.05 * std::pow(m, 0.94);
    return 1.37 * std::pow(m, 0.54);
}

// L = 4 pi R^2 sigma T^4  =>  T / T_sun = (L / R^2)^(1/4) in solar units.
double effectiveTemperature(double luminositySolar, double radiusSolar) {
    return kSolarTeffK * std::pow(luminositySolar / (radiusSolar * radiusSolar), 0.25);
}

// Fuel ~ M, burn rate ~ L, normalized to ~1e10 yr for the Sun. Known to
// overestimate the lifetimes of the most massive stars; order of magnitude only.
double mainSequenceLifetimeYears(double massSolar, double luminositySolar) {
    return 1.0e10 * massSolar / luminositySolar;
}

MainSequenceProperties mainSequenceProperties(double massSolar) {
    const double m = std::clamp(massSolar, kMinMassSolar, kMaxMassSolar);
    MainSequenceProperties p{};
    p.massSolar       = m;
    p.luminositySolar = luminosityFromMass(m);
    p.radiusSolar     = radiusFromMass(m);
    p.teffK           = effectiveTemperature(p.luminositySolar, p.radiusSolar);
    p.lifetimeYears   = mainSequenceLifetimeYears(m, p.luminositySolar);
    return p;
}

namespace {
// Piecewise Gaussian used by the multi-lobe CIE 1931 fit of
// Wyman, Sloan & Shirley (2013), "Simple Analytic Approximations to the CIE XYZ
// Color Matching Functions", JCGT 2(2).
double lobe(double x, double mu, double sigmaLow, double sigmaHigh) {
    const double s = (x - mu) / (x < mu ? sigmaLow : sigmaHigh);
    return std::exp(-0.5 * s * s);
}

// Planck spectral radiance, arbitrary constant factor (only ratios matter).
double planck(double lambdaNm, double temperatureK) {
    constexpr double c2 = 1.438776877e-2; // second radiation constant h c / k_B [m K]
    const double lambdaM = lambdaNm * 1e-9;
    return 1.0 / (std::pow(lambdaM, 5.0) * (std::exp(c2 / (lambdaM * temperatureK)) - 1.0));
}
} // namespace

glm::vec3 blackbodyColor(double temperatureK) {
    const double T = std::clamp(temperatureK, 800.0, 100000.0);
    double X = 0.0, Y = 0.0, Z = 0.0;
    for (double l = 380.0; l <= 780.0; l += 5.0) {
        const double b = planck(l, T);
        X += b * (1.056 * lobe(l, 599.8, 37.9, 31.0) + 0.362 * lobe(l, 442.0, 16.0, 26.7)
                  - 0.065 * lobe(l, 501.1, 20.4, 26.2));
        Y += b * (0.821 * lobe(l, 568.8, 46.9, 40.5) + 0.286 * lobe(l, 530.9, 16.3, 31.1));
        Z += b * (1.217 * lobe(l, 437.0, 11.8, 36.0) + 0.681 * lobe(l, 459.0, 26.0, 13.8));
    }
    // CIE XYZ -> linear sRGB (D65).
    double r =  3.2406 * X - 1.5372 * Y - 0.4986 * Z;
    double g = -0.9689 * X + 1.8758 * Y + 0.0415 * Z;
    double b =  0.0557 * X - 0.2040 * Y + 1.0570 * Z;
    r = std::max(r, 0.0); g = std::max(g, 0.0); b = std::max(b, 0.0);
    const double peak = std::max({r, g, b, 1e-30});
    return glm::vec3(static_cast<float>(r / peak), static_cast<float>(g / peak),
                     static_cast<float>(b / peak));
}

double freeFallTimeYears(double numberDensityH2PerCm3) {
    constexpr double kPi         = 3.14159265358979323846;
    constexpr double G           = 6.67430e-11;   // m^3 kg^-1 s^-2 (CODATA 2018)
    constexpr double kHydrogenKg = 1.6735575e-27; // mass of a hydrogen atom
    constexpr double kMu         = 2.8;           // mean mass per H2 molecule incl. He
    constexpr double kYear       = 3.15576e7;     // Julian year in seconds
    const double rho = numberDensityH2PerCm3 * 1e6 * kMu * kHydrogenKg; // kg m^-3
    return std::sqrt(3.0 * kPi / (32.0 * G * rho)) / kYear;
}

} // namespace stellaris
