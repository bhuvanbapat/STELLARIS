#include "test_harness.h"

#include "sim/StellarModel.h"

using namespace stellaris;

TEST(mass_luminosity_relation_pieces) {
    CHECK_NEAR(luminosityFromMass(1.0), 1.0, 1e-12);
    CHECK_NEAR(luminosityFromMass(10.0), 1.4 * std::pow(10.0, 3.5), 1e-6);
    CHECK_NEAR(luminosityFromMass(60.0), 32000.0 * 60.0, 1e-6);
    // Monotonic over the selectable range.
    double prev = 0.0;
    for (double m = kMinMassSolar; m <= kMaxMassSolar; m += 0.25) {
        const double l = luminosityFromMass(m);
        CHECK(l > prev);
        prev = l;
    }
}

TEST(sun_like_star_is_close_to_solar_values) {
    const MainSequenceProperties p = mainSequenceProperties(1.0);
    CHECK_NEAR(p.luminositySolar, 1.0, 1e-12);
    CHECK_NEAR(p.radiusSolar, 1.05, 1e-12);          // empirical fit, not exactly 1
    CHECK(std::fabs(p.teffK - kSolarTeffK) / kSolarTeffK < 0.05);
    CHECK_NEAR(p.lifetimeYears, 1.0e10, 1.0);
}

TEST(massive_stars_are_hotter_brighter_shorter_lived) {
    const MainSequenceProperties a = mainSequenceProperties(1.0);
    const MainSequenceProperties b = mainSequenceProperties(15.0);
    const MainSequenceProperties c = mainSequenceProperties(40.0);
    CHECK(b.teffK > a.teffK && c.teffK > b.teffK);
    CHECK(b.luminositySolar > a.luminositySolar && c.luminositySolar > b.luminositySolar);
    CHECK(b.lifetimeYears < a.lifetimeYears && c.lifetimeYears < b.lifetimeYears);
    // Plausible order of magnitude for a ~15 Msun B-type star.
    CHECK(b.teffK > 20000.0 && b.teffK < 40000.0);
}

TEST(mass_is_clamped_to_supported_range) {
    CHECK(mainSequenceProperties(0.1).massSolar == kMinMassSolar);
    CHECK(mainSequenceProperties(500.0).massSolar == kMaxMassSolar);
}

TEST(pathway_categories) {
    CHECK(pathwayForMass(1.0) == Pathway::WhiteDwarf);
    CHECK(pathwayForMass(7.99) == Pathway::WhiteDwarf);
    CHECK(pathwayForMass(8.0) == Pathway::NeutronStar);
    CHECK(pathwayForMass(24.99) == Pathway::NeutronStar);
    CHECK(pathwayForMass(25.0) == Pathway::BlackHole);
    CHECK(pathwayForMass(40.0) == Pathway::BlackHole);
    CHECK(massNearCategoryBoundary(8.0));
    CHECK(massNearCategoryBoundary(25.0));
    CHECK(!massNearCategoryBoundary(1.0));
    CHECK(!massNearCategoryBoundary(14.0));
    CHECK(!massNearCategoryBoundary(40.0));
}

TEST(blackbody_colors_follow_temperature) {
    const glm::vec3 cool = blackbodyColor(3000.0);
    const glm::vec3 sun = blackbodyColor(5772.0);
    const glm::vec3 hot = blackbodyColor(30000.0);
    CHECK(cool.r > cool.g && cool.g > cool.b);  // orange-red
    CHECK(hot.b > hot.r);                       // blue-white
    CHECK(sun.b > 0.6f && sun.r > 0.9f);        // near white
    for (float c : {cool.r, cool.g, cool.b, hot.r, hot.g, hot.b}) CHECK(c >= 0.0f && c <= 1.0f);
}

TEST(free_fall_time_order_of_magnitude) {
    const double t = freeFallTimeYears(1e4);
    CHECK(t > 2e5 && t < 5e5);
    // t_ff scales as n^-1/2.
    CHECK_NEAR(freeFallTimeYears(4e4), t / 2.0, t * 1e-9);
}

TEST(comparison_masses_are_in_domain_and_ordered) {
    double prevL = 0.0, prevT = 0.0, prevLife = 1e30;
    for (double m : kComparisonMasses) {
        CHECK(m >= kMinMassSolar && m <= kMaxMassSolar);
        const MainSequenceProperties p = mainSequenceProperties(m);
        CHECK(p.luminositySolar > prevL && p.teffK > prevT && p.lifetimeYears < prevLife);
        prevL = p.luminositySolar; prevT = p.teffK; prevLife = p.lifetimeYears;
    }
    // Every final pathway is represented in the table.
    bool seen[3] = {};
    for (double m : kComparisonMasses) seen[static_cast<int>(pathwayForMass(m))] = true;
    CHECK(seen[0] && seen[1] && seen[2]);
}
