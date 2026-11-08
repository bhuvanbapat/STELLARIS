#include "sim/CloudField.h"

#include <glm/geometric.hpp>

#include <algorithm>
#include <cmath>
#include <random>
#include <thread>

namespace stellaris {

namespace {

constexpr float kTwoPi = 6.28318530718f;

// mt19937 is fully specified; the conversions below avoid the standard library's
// implementation-defined distributions so results match on every compiler.
struct Rng {
    std::mt19937 engine;
    explicit Rng(uint32_t seed) : engine(seed) {}
    float uniform() { return static_cast<float>(engine() >> 8) * (1.0f / 16777216.0f); }
    float range(float a, float b) { return a + (b - a) * uniform(); }
    glm::vec3 unitVector() {
        const float z = range(-1.0f, 1.0f), a = range(0.0f, kTwoPi), s = std::sqrt(1.0f - z * z);
        return {s * std::cos(a), z, s * std::sin(a)};
    }
};

uint32_t hash3(int x, int y, int z, uint32_t seed) {
    uint32_t h = seed * 0x9E3779B9u;
    h ^= static_cast<uint32_t>(x) * 0x85EBCA6Bu;
    h = (h ^ (h >> 13)) * 0xC2B2AE35u;
    h ^= static_cast<uint32_t>(y) * 0x27D4EB2Fu;
    h = (h ^ (h >> 15)) * 0x165667B1u;
    h ^= static_cast<uint32_t>(z) * 0x9E3779B1u;
    h = (h ^ (h >> 16)) * 0x85EBCA77u;
    return h ^ (h >> 13);
}

float smooth01(float e0, float e1, float x) {
    const float t = std::clamp((x - e0) / (e1 - e0), 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}

float fbm2(const glm::vec3& p, uint32_t seed) {
    return 0.62f * valueNoise3(p, seed) + 0.38f * valueNoise3(p * 2.03f + glm::vec3(5.1f), seed + 17u);
}

// Squared distance from p to segment ab and the parameter of the closest point.
float segmentDistance2(const glm::vec3& p, const glm::vec3& a, const glm::vec3& b, float& t) {
    const glm::vec3 ab = b - a;
    t = std::clamp(glm::dot(p - a, ab) / std::max(glm::dot(ab, ab), 1e-6f), 0.0f, 1.0f);
    const glm::vec3 d = p - (a + ab * t);
    return glm::dot(d, d);
}

glm::vec3 bezier(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, float t) {
    const float u = 1.0f - t;
    return u * u * u * p0 + 3.0f * u * u * t * p1 + 3.0f * u * t * t * p2 + t * t * t * p3;
}

glm::vec3 anyPerpendicular(const glm::vec3& d) {
    const glm::vec3 ref = std::abs(d.y) < 0.9f ? glm::vec3(0, 1, 0) : glm::vec3(1, 0, 0);
    return glm::normalize(glm::cross(d, ref));
}

} // namespace

float valueNoise3(const glm::vec3& p, uint32_t seed) {
    const glm::vec3 fl(std::floor(p.x), std::floor(p.y), std::floor(p.z));
    const glm::vec3 f = p - fl;
    const glm::vec3 u = f * f * (3.0f - 2.0f * f);
    const int x = static_cast<int>(fl.x), y = static_cast<int>(fl.y), z = static_cast<int>(fl.z);
    auto v = [&](int dx, int dy, int dz) {
        return static_cast<float>(hash3(x + dx, y + dy, z + dz, seed) >> 8) * (1.0f / 16777216.0f);
    };
    const float x00 = v(0, 0, 0) + (v(1, 0, 0) - v(0, 0, 0)) * u.x;
    const float x10 = v(0, 1, 0) + (v(1, 1, 0) - v(0, 1, 0)) * u.x;
    const float x01 = v(0, 0, 1) + (v(1, 0, 1) - v(0, 0, 1)) * u.x;
    const float x11 = v(0, 1, 1) + (v(1, 1, 1) - v(0, 1, 1)) * u.x;
    const float y0 = x00 + (x10 - x00) * u.y;
    const float y1 = x01 + (x11 - x01) * u.y;
    return y0 + (y1 - y0) * u.z;
}

glm::vec3 CloudField::keyLightDirection() { return glm::normalize(glm::vec3(-0.45f, 0.55f, -0.70f)); }

CloudField CloudField::build(uint32_t seed, int resolution) {
    CloudField f;
    f.seed_ = seed;
    f.resolution_ = std::max(resolution, 8);
    Rng rng(seed * 2654435761u + 12345u);
    f.noiseOffset_ = glm::vec3(rng.range(0, 100), rng.range(0, 100), rng.range(0, 100));

    // --- Envelope: soft union of anisotropic blobs (the first one is centered).
    for (int i = 0; i < 7; ++i) {
        Blob b;
        b.center = i == 0 ? glm::vec3(0.0f)
                          : glm::vec3(rng.range(-32, 32), rng.range(-9, 9), rng.range(-20, 20));
        const glm::vec3 radii = i == 0 ? glm::vec3(24, 13, 19)
                                       : glm::vec3(rng.range(11, 20), rng.range(7, 12), rng.range(9, 16));
        b.inverseRadii = 1.0f / radii;
        b.strength = i == 0 ? 1.1f : rng.range(0.6f, 1.0f);
        f.envelopeBlobs_.push_back(b);
    }

    // --- Hub-filament network: filaments lie roughly in a sheet and converge on the hub.
    const glm::vec3 sheetNormal = glm::normalize(glm::vec3(rng.range(-0.25f, 0.25f), 1.0f, rng.range(-0.3f, 0.3f)));
    const glm::vec3 su = anyPerpendicular(sheetNormal);
    const glm::vec3 sv = glm::cross(sheetNormal, su);
    const int mainCount = 5;
    auto addFilament = [&](glm::vec3 start, glm::vec3 dir, float length, float w0, float w1, float strength, int samples) {
        Filament fil;
        const glm::vec3 side = anyPerpendicular(dir);
        const glm::vec3 up = glm::cross(dir, side);
        auto lateral = [&](float mag) { return side * rng.range(-mag, mag) + up * rng.range(-mag * 0.5f, mag * 0.5f); };
        const glm::vec3 p0 = start;
        const glm::vec3 p1 = start + dir * (length / 3.0f) + lateral(6.0f);
        const glm::vec3 p2 = start + dir * (2.0f * length / 3.0f) + lateral(8.0f);
        const glm::vec3 p3 = start + dir * length + lateral(7.0f);
        fil.boundsMin = glm::vec3(1e9f);
        fil.boundsMax = glm::vec3(-1e9f);
        for (int s = 0; s < samples; ++s) {
            const float t = static_cast<float>(s) / static_cast<float>(samples - 1);
            const glm::vec3 p = bezier(p0, p1, p2, p3, t);
            const float w = (w0 + (w1 - w0) * t) * (0.8f + 0.4f * valueNoise3(p * 0.15f, seed + 3u));
            fil.points.push_back(p);
            fil.widths.push_back(w);
            fil.boundsMin = glm::min(fil.boundsMin, p - glm::vec3(5.0f * w));
            fil.boundsMax = glm::max(fil.boundsMax, p + glm::vec3(5.0f * w));
        }
        fil.strength = strength;
        f.filaments_.push_back(std::move(fil));
    };
    for (int k = 0; k < mainCount; ++k) {
        const float angle = kTwoPi * static_cast<float>(k) / mainCount + rng.range(-0.35f, 0.35f);
        const glm::vec3 dir = glm::normalize(std::cos(angle) * su + std::sin(angle) * sv + sheetNormal * rng.range(-0.25f, 0.25f));
        addFilament(dir * 3.0f, dir, rng.range(30.0f, 46.0f), 2.7f, 1.3f, 1.0f, 24);
    }
    // Side branches leaving two of the main filaments.
    for (int b = 0; b < 2; ++b) {
        const Filament& parent = f.filaments_[static_cast<size_t>(rng.uniform() * mainCount) % mainCount];
        const size_t idx = static_cast<size_t>(rng.range(0.45f, 0.6f) * static_cast<float>(parent.points.size()));
        const glm::vec3 origin = parent.points[idx];
        const glm::vec3 tangent = glm::normalize(parent.points[idx + 1] - parent.points[idx - 1]);
        const float turn = rng.range(0.7f, 1.2f) * (rng.uniform() < 0.5f ? -1.0f : 1.0f);
        const glm::vec3 inPlane = glm::normalize(glm::cross(sheetNormal, tangent));
        const glm::vec3 dir = glm::normalize(tangent * std::cos(turn) + inPlane * std::sin(turn));
        addFilament(origin, dir, rng.range(12.0f, 18.0f), 1.6f, 0.9f, 0.8f, 12);
    }

    // --- Secondary condensations sitting on filaments.
    for (int c = 0; c < 3; ++c) {
        const Filament& fil = f.filaments_[static_cast<size_t>(c * 2) % mainCount];
        const size_t idx = static_cast<size_t>(rng.range(0.4f, 0.7f) * static_cast<float>(fil.points.size()));
        f.condensations_.push_back({fil.points[idx], rng.range(2.2f, 3.2f), rng.range(0.7f, 0.9f)});
    }

    // --- Globules: compact dense knots on the camera-facing side of the envelope.
    for (int g = 0; g < 4; ++g) {
        const glm::vec3 dir = glm::normalize(rng.unitVector() * 0.6f + glm::vec3(-0.3f, 0.1f, 0.9f));
        f.globules_.push_back({dir * rng.range(18.0f, 30.0f), rng.range(1.6f, 3.0f), 1.0f});
    }

    // --- Bake the grid (z slices in parallel).
    const int n = f.resolution_;
    f.voxels_.assign(static_cast<size_t>(n) * n * n * 4, 0.0f);
    const unsigned threads = std::max(1u, std::min(16u, std::thread::hardware_concurrency()));
    std::vector<std::thread> pool;
    for (unsigned t = 0; t < threads; ++t) {
        pool.emplace_back([&f, n, t, threads] {
            const float cell = 2.0f * kExtent / static_cast<float>(n);
            for (int z = static_cast<int>(t); z < n; z += static_cast<int>(threads)) {
                for (int y = 0; y < n; ++y) {
                    for (int x = 0; x < n; ++x) {
                        const glm::vec3 p(-kExtent + (x + 0.5f) * cell, -kExtent + (y + 0.5f) * cell,
                                          -kExtent + (z + 0.5f) * cell);
                        const CloudComponents c = f.evaluate(p);
                        float* v = &f.voxels_[((static_cast<size_t>(z) * n + y) * n + x) * 4];
                        v[0] = c.envelope; v[1] = c.filament; v[2] = c.core; v[3] = c.globule;
                    }
                }
            }
        });
    }
    for (std::thread& th : pool) th.join();
    return f;
}

CloudComponents CloudField::evaluate(const glm::vec3& p) const {
    CloudComponents c;

    // Envelope: soft union, broken up by low-frequency modulation and voids.
    float sum = 0.0f;
    for (const Blob& b : envelopeBlobs_) {
        const glm::vec3 q = (p - b.center) * b.inverseRadii;
        sum += b.strength * std::exp(-glm::dot(q, q));
    }
    float env = 1.0f - std::exp(-1.6f * sum);
    const float modulation = fbm2(p * 0.035f + noiseOffset_, seed_);
    const float voids = smooth01(0.60f, 0.76f, fbm2(p * 0.055f + noiseOffset_ * 1.7f, seed_ + 101u));
    env *= (0.45f + 0.9f * modulation) * (1.0f - 0.85f * voids);
    c.envelope = std::clamp(env, 0.0f, 1.2f);

    // Filaments: Plummer-like transverse profile (1 + (d/w)^2)^-1.5 around each centerline.
    float fil = 0.0f;
    for (const Filament& f : filaments_) {
        if (p.x < f.boundsMin.x || p.y < f.boundsMin.y || p.z < f.boundsMin.z ||
            p.x > f.boundsMax.x || p.y > f.boundsMax.y || p.z > f.boundsMax.z) continue;
        const size_t count = f.points.size();
        for (size_t i = 0; i + 1 < count; ++i) {
            float t = 0.0f;
            const float d2 = segmentDistance2(p, f.points[i], f.points[i + 1], t);
            const float w = f.widths[i] + (f.widths[i + 1] - f.widths[i]) * t;
            if (d2 > 25.0f * w * w) continue;
            const float s = (static_cast<float>(i) + t) / static_cast<float>(count - 1);
            const float taper = (1.0f - smooth01(0.8f, 1.0f, s)) * (0.7f + 0.3f * (1.0f - s));
            const float x = d2 / (w * w);
            fil = std::max(fil, f.strength * taper / ((1.0f + x) * std::sqrt(1.0f + x)));
        }
    }
    c.filament = fil;

    // Dense core at the hub and condensations along filaments.
    const float r = glm::length(p);
    float core = (1.0f / (1.0f + (r / 4.5f) * (r / 4.5f))) * (1.0f - smooth01(10.0f, 16.0f, r));
    for (const Clump& k : condensations_) {
        const float d = glm::length(p - k.center);
        core = std::max(core, k.strength * (1.0f - smooth01(2.0f * k.radius, 3.0f * k.radius, d)) /
                                  (1.0f + (d / k.radius) * (d / k.radius)));
    }
    c.core = core;

    // Globules: compact, irregular knots with a cometary tail pointing away from the
    // illuminating source (the shape seen in real cometary globules; illustrative).
    float glob = 0.0f;
    const glm::vec3 away = -keyLightDirection();
    for (const Clump& g : globules_) {
        const glm::vec3 q = p - g.center;
        const float along = glm::dot(q, away);
        const float tail = std::max(along, 0.0f);
        // Squeeze the down-stream half so the knot stretches into a tapering tail.
        const glm::vec3 shaped = q - away * (tail * 0.7f);
        const float wobble = 0.75f + 0.5f * valueNoise3(q * 0.6f + g.center, seed_ + 211u);
        const float d = glm::length(shaped) / (g.radius * wobble * (1.0f - 0.35f * std::min(tail / (3.0f * g.radius), 1.0f)));
        if (d < 1.0f) glob = std::max(glob, g.strength * std::pow(1.0f - d * d, 1.5f));
    }
    c.globule = glob;
    return c;
}

CloudComponents CloudField::sample(const glm::vec3& p) const {
    const int n = resolution_;
    const glm::vec3 g = (p + glm::vec3(kExtent)) / (2.0f * kExtent) * static_cast<float>(n) - 0.5f;
    const int x0 = static_cast<int>(std::floor(g.x)), y0 = static_cast<int>(std::floor(g.y)),
              z0 = static_cast<int>(std::floor(g.z));
    const glm::vec3 f = g - glm::vec3(x0, y0, z0);
    float out[4] = {0, 0, 0, 0};
    for (int k = 0; k < 8; ++k) {
        const int x = x0 + (k & 1), y = y0 + ((k >> 1) & 1), z = z0 + ((k >> 2) & 1);
        if (x < 0 || y < 0 || z < 0 || x >= n || y >= n || z >= n) continue;  // outside = empty
        const float w = ((k & 1) ? f.x : 1 - f.x) * (((k >> 1) & 1) ? f.y : 1 - f.y) * (((k >> 2) & 1) ? f.z : 1 - f.z);
        const float* v = &voxels_[((static_cast<size_t>(z) * n + y) * n + x) * 4];
        for (int c = 0; c < 4; ++c) out[c] += w * v[c];
    }
    return {out[0], out[1], out[2], out[3]};
}

} // namespace stellaris
