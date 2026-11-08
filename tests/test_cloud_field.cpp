#include "test_harness.h"

#include "render/ParticleSystem.h"
#include "sim/CloudField.h"

#include <glm/geometric.hpp>

#include <cstring>

using namespace stellaris;

namespace {
float total(const CloudComponents& c) { return c.envelope + c.filament + c.core + c.globule; }
} // namespace

TEST(cloud_field_same_seed_is_bitwise_identical) {
    const CloudField a = CloudField::build(7u, 32);
    const CloudField b = CloudField::build(7u, 32);
    CHECK(a.voxels().size() == b.voxels().size());
    CHECK(std::memcmp(a.voxels().data(), b.voxels().data(), a.voxels().size() * sizeof(float)) == 0);
}

TEST(cloud_field_different_seeds_differ) {
    const CloudField a = CloudField::build(1u, 32);
    const CloudField b = CloudField::build(2u, 32);
    double diff = 0.0;
    for (size_t i = 0; i < a.voxels().size(); ++i) diff += std::fabs(a.voxels()[i] - b.voxels()[i]);
    CHECK(diff / static_cast<double>(a.voxels().size()) > 0.005);
}

TEST(cloud_field_values_are_finite_and_bounded) {
    for (uint32_t seed : {1u, 2u, 3u, 99u}) {
        const CloudField f = CloudField::build(seed, 24);
        float maxValue = 0.0f;
        for (float v : f.voxels()) {
            CHECK(std::isfinite(v) && v >= 0.0f);
            maxValue = std::max(maxValue, v);
        }
        CHECK(maxValue <= 1.25f);
        CHECK(maxValue > 0.5f);  // not empty
    }
}

TEST(cloud_field_dense_core_is_identifiable) {
    for (uint32_t seed : {1u, 5u, 12u}) {
        const CloudField f = CloudField::build(seed, 16);
        const CloudComponents center = f.evaluate(glm::vec3(0.0f));
        CHECK(center.core > 0.95f);
        // The core is much denser than the average over a sphere of radius 30.
        double shell = 0.0;
        int count = 0;
        for (int i = 0; i < 200; ++i) {
            const float z = -1.0f + 2.0f * (i + 0.5f) / 200.0f, a = 2.399963f * i, s = std::sqrt(1.0f - z * z);
            shell += total(f.evaluate(glm::vec3(s * std::cos(a), z, s * std::sin(a)) * 30.0f));
            ++count;
        }
        CHECK(total(center) > 2.0 * shell / count);
    }
}

TEST(cloud_field_filaments_follow_their_centerlines) {
    const CloudField f = CloudField::build(3u, 16);
    CHECK(f.filamentCount() >= 5);
    for (int i = 0; i < f.filamentCount(); ++i) {
        const auto& path = f.filamentPath(i);
        const glm::vec3 mid = path[path.size() / 3];
        CHECK(f.evaluate(mid).filament > 0.55f);
        // Well away from the centerline (10 units sideways) the filament term is small.
        const glm::vec3 tangent = glm::normalize(path[path.size() / 3 + 1] - path[path.size() / 3 - 1]);
        const glm::vec3 side = glm::normalize(glm::cross(tangent, std::abs(tangent.y) < 0.9f ? glm::vec3(0, 1, 0) : glm::vec3(1, 0, 0)));
        CHECK(f.evaluate(mid + side * 10.0f).filament < f.evaluate(mid).filament * 0.5f);
    }
}

TEST(cloud_field_bake_matches_exact_evaluation) {
    const CloudField f = CloudField::build(4u, 32);
    const float cell = 2.0f * CloudField::kExtent / 32.0f;
    for (int k = 0; k < 50; ++k) {
        const int x = (k * 7) % 32, y = (k * 13) % 32, z = (k * 5) % 32;
        const glm::vec3 p(-CloudField::kExtent + (x + 0.5f) * cell, -CloudField::kExtent + (y + 0.5f) * cell,
                          -CloudField::kExtent + (z + 0.5f) * cell);
        const CloudComponents a = f.evaluate(p), b = f.sample(p);
        CHECK_NEAR(a.envelope, b.envelope, 1e-4);
        CHECK_NEAR(a.filament, b.filament, 1e-4);
        CHECK_NEAR(a.core, b.core, 1e-4);
        CHECK_NEAR(a.globule, b.globule, 1e-4);
    }
    // Outside the grid the field is empty.
    CHECK(total(f.sample(glm::vec3(500.0f))) == 0.0f);
}

TEST(ambient_particles_follow_the_cloud_field) {
    const CloudField f = CloudField::build(1u, 32);
    ParticleCounts counts;
    counts.ambient = 3000; counts.core = 0; counts.jet = 0; counts.ejecta = 0; counts.wind = 0;
    const auto particles = generateParticles(counts, 11u, f);
    double onParticles = 0.0;
    for (const ParticleVertex& v : particles) onParticles += f.sample({v.pos[0], v.pos[1], v.pos[2]}).envelope +
                                                             f.sample({v.pos[0], v.pos[1], v.pos[2]}).filament;
    // Mean field value at uniformly random points in the same box, for comparison.
    double uniform = 0.0;
    uint32_t s = 1u;
    for (int i = 0; i < 3000; ++i) {
        auto r = [&] { s = s * 1664525u + 1013904223u; return (s >> 8) * (1.0f / 16777216.0f) * 2.0f - 1.0f; };
        const glm::vec3 p(r() * 53.0f, r() * 32.0f, r() * 42.0f);
        uniform += f.sample(p).envelope + f.sample(p).filament;
    }
    CHECK(static_cast<int>(particles.size()) == 3000);
    CHECK(onParticles > 2.0 * uniform);
}
