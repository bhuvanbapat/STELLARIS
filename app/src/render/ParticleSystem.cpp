#include "render/ParticleSystem.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <random>

namespace stellaris {

namespace {

constexpr float kCoreRadius = 13.0f;  // must match R_CORE in particles.vert

// Explicit conversion so results do not depend on the standard library's
// distribution implementations (mt19937 itself is fully specified).
struct Rng {
    std::mt19937 engine;
    explicit Rng(uint32_t seed) : engine(seed) {}
    float uniform() { return static_cast<float>(engine() >> 8) * (1.0f / 16777216.0f); }
    float range(float a, float b) { return a + (b - a) * uniform(); }
};

// CPU twin of cloudShape() in shaders/volume.frag (large-scale structure only),
// used so ambient particles follow the same filament and clumps as the volume.
float cloudShape(float x, float y, float z) {
    const float ax = 1.0f, ay = 0.22f, az = -0.38f;
    const float an = std::sqrt(ax * ax + ay * ay + az * az);
    const float s = (x * ax + y * ay + z * az) / an;
    float px = x - ax / an * s, py = y - ay / an * s, pz = z - az / an * s;
    py += 4.0f * std::sin(s * 0.06f);
    pz += 3.0f * std::cos(s * 0.045f + 1.0f);
    const float width = 9.0f + 3.0f * std::sin(s * 0.09f + 2.0f);
    const float d2 = px * px + py * py + pz * pz;
    const float filament = std::exp(-d2 / (2.0f * width * width)) * std::exp(-(s / 44.0f) * (s / 44.0f));
    const float core = std::exp(-(x * x + y * y + z * z) / (2.0f * 7.5f * 7.5f));
    auto blob = [&](float cx, float cy, float cz, float k) {
        const float dx = x - cx, dy = y - cy, dz = z - cz;
        return std::exp(-(dx * dx + dy * dy + dz * dz) / k);
    };
    const float clumps = std::max({0.7f * blob(-20, 7, 10, 110), 0.6f * blob(22, -6, -8, 90), 0.35f * blob(6, 12, 18, 70)});
    return std::max(filament * 0.85f + core * 0.9f, clumps);
}

void push(std::vector<ParticleVertex>& out, float x, float y, float z, int population, Rng& rng) {
    ParticleVertex v{};
    v.pos[0] = x; v.pos[1] = y; v.pos[2] = z; v.pos[3] = static_cast<float>(population);
    for (float& s : v.seed) s = rng.uniform();
    out.push_back(v);
}

} // namespace

std::vector<ParticleVertex> generateParticles(const ParticleCounts& counts, uint32_t seed) {
    Rng rng(seed);
    std::vector<ParticleVertex> out;
    out.reserve(static_cast<size_t>(counts.total()));

    // Ambient cloud gas: rejection-sampled from the cloud shape, outside the core.
    for (int n = 0; n < counts.ambient;) {
        const float x = rng.range(-48, 48), y = rng.range(-30, 30), z = rng.range(-40, 40);
        const float r = std::sqrt(x * x + y * y + z * z);
        if (r < kCoreRadius * 0.85f) continue;
        const float p = cloudShape(x, y, z);
        if (rng.uniform() > p * p * 1.3f) continue;
        push(out, x, y, z, 0, rng);
        ++n;
    }
    // Dense core: centrally concentrated (roughly rho ~ r^-1.5 inside R_core).
    for (int n = 0; n < counts.core;) {
        const float x = rng.range(-1, 1), y = rng.range(-1, 1), z = rng.range(-1, 1);
        const float l2 = x * x + y * y + z * z;
        if (l2 > 1.0f || l2 < 1e-6f) continue;
        const float rr = std::pow(rng.uniform(), 0.5f) * kCoreRadius;  // centrally concentrated
        const float inv = rr / std::sqrt(l2);
        push(out, x * inv, y * inv * 0.85f, z * inv, 1, rng);
        ++n;
    }
    for (int n = 0; n < counts.jet; ++n) push(out, 0, 0, 0, 2, rng);
    for (int n = 0; n < counts.ejecta; ++n) push(out, 0, 0, 0, 3, rng);
    return out;
}

void ParticleSystem::init(const ParticleCounts& counts, uint32_t seed) {
    const std::vector<ParticleVertex> data = generateParticles(counts, seed);
    count_ = static_cast<int>(data.size());

    vao_ = gl::createVertexArray();
    vbo_ = gl::createBuffer();
    glNamedBufferStorage(vbo_.id(), static_cast<GLsizeiptr>(data.size() * sizeof(ParticleVertex)), data.data(), 0);
    glVertexArrayVertexBuffer(vao_.id(), 0, vbo_.id(), 0, sizeof(ParticleVertex));
    glEnableVertexArrayAttrib(vao_.id(), 0);
    glVertexArrayAttribFormat(vao_.id(), 0, 4, GL_FLOAT, GL_FALSE, offsetof(ParticleVertex, pos));
    glVertexArrayAttribBinding(vao_.id(), 0, 0);
    glEnableVertexArrayAttrib(vao_.id(), 1);
    glVertexArrayAttribFormat(vao_.id(), 1, 4, GL_FLOAT, GL_FALSE, offsetof(ParticleVertex, seed));
    glVertexArrayAttribBinding(vao_.id(), 1, 0);
}

void ParticleSystem::draw() const {
    glBindVertexArray(vao_.id());
    glDrawArrays(GL_POINTS, 0, count_);
}

} // namespace stellaris
