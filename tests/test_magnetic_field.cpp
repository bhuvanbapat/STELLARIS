#include "test_harness.h"

#include "sim/MagneticField.h"

#include <glm/geometric.hpp>
#include <glm/trigonometric.hpp>

using namespace stellaris;

TEST(dipole_field_matches_hand_computed_values) {
    const glm::vec3 m(0.0f, 1.0f, 0.0f);
    // On the axis at r = 2: (1/8)[3*1*y_hat - y_hat] = (0, 0.25, 0).
    const glm::vec3 axis = dipoleField({0.0f, 2.0f, 0.0f}, m);
    CHECK_NEAR(axis.x, 0.0, 1e-7); CHECK_NEAR(axis.y, 0.25, 1e-7); CHECK_NEAR(axis.z, 0.0, 1e-7);
    // On the equator at r = 2: (1/8)[0 - y_hat] = (0, -0.125, 0): antiparallel to m.
    const glm::vec3 eq = dipoleField({2.0f, 0.0f, 0.0f}, m);
    CHECK_NEAR(eq.y, -0.125, 1e-7); CHECK_NEAR(eq.x, 0.0, 1e-7);
    // Strength falls off as r^-3.
    CHECK_NEAR(glm::length(dipoleField({0, 4, 0}, m)), glm::length(dipoleField({0, 2, 0}, m)) / 8.0, 1e-7);
    // Singular point handled.
    CHECK(glm::length(dipoleField({0, 0, 0}, m)) == 0.0f);
}

TEST(dipole_field_lines_follow_r_equals_L_sin2_theta) {
    // Closed dipole field lines satisfy r = L sin^2(theta), theta from the axis.
    const glm::vec3 m(0.0f, 1.0f, 0.0f);
    FieldLineSettings s;
    s.maxRadius = 20.0f;
    for (float latDeg : {30.0f, 45.0f, 60.0f}) {
        const float lat = glm::radians(latDeg);
        const glm::vec3 start(std::cos(lat), std::sin(lat), 0.0f);
        const auto line = traceFieldLine(start, m, s);
        CHECK(line.size() > 20);
        const float L = 1.0f / (std::cos(lat) * std::cos(lat));  // sin(theta) = cos(lat) at r = 1
        float worst = 0.0f;  // every point, including the surface end point
        for (const glm::vec3& p : line) {
            const float r = glm::length(p);
            const float sinTheta = std::sqrt(p.x * p.x + p.z * p.z) / r;
            worst = std::max(worst, std::fabs(r / (L * sinTheta * sinTheta) - 1.0f));
        }
        CHECK(worst < 0.002f);
        // The line closes on the opposite hemisphere at the mirrored latitude.
        const glm::vec3 end = line.back();
        CHECK_NEAR(glm::length(end), 1.0, 1e-4);
        CHECK_NEAR(end.y, -start.y, 0.002);
        // Apex on the magnetic equator at r = L.
        float apex = 0.0f;
        for (const glm::vec3& p : line) apex = std::max(apex, glm::length(p));
        CHECK_NEAR(apex, L, 0.02 * L);
    }
}

TEST(field_lines_are_deterministic_and_bounded) {
    FieldLineSettings s;
    const glm::vec3 m = glm::normalize(glm::vec3(0.3f, 1.0f, 0.1f));
    const auto a = traceDipoleLines(m, {35.0f, 55.0f, 75.0f}, 8, s);
    const auto b = traceDipoleLines(m, {35.0f, 55.0f, 75.0f}, 8, s);
    CHECK(a.size() == 24 && b.size() == 24);
    for (size_t i = 0; i < a.size(); ++i) {
        CHECK(a[i].size() == b[i].size());
        CHECK(a[i].size() <= static_cast<size_t>(s.maxSteps) + 2);
        for (size_t k = 0; k < a[i].size(); ++k) {
            CHECK(a[i][k] == b[i][k]);
            CHECK(glm::length(a[i][k]) <= s.maxRadius + s.step * 2.0f);
        }
    }
    // High-latitude footpoints give open lines cut at maxRadius.
    float maxR = 0.0f;
    for (const glm::vec3& p : a.back()) maxR = std::max(maxR, glm::length(p));
    CHECK(maxR > s.maxRadius);
}
