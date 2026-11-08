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

void push(std::vector<ParticleVertex>& out, float x, float y, float z, int population, Rng& rng) {
    ParticleVertex v{};
    v.pos[0] = x; v.pos[1] = y; v.pos[2] = z; v.pos[3] = static_cast<float>(population);
    for (float& s : v.seed) s = rng.uniform();
    out.push_back(v);
}

} // namespace

std::vector<ParticleVertex> generateParticles(const ParticleCounts& counts, uint32_t seed, const CloudField& cloud) {
    Rng rng(seed);
    std::vector<ParticleVertex> out;
    out.reserve(static_cast<size_t>(counts.total()));

    // Ambient cloud gas: rejection-sampled from the same envelope + filament field the
    // volume renderer uses, outside the collapsing core. The attempt cap only guards
    // against a degenerate (empty) field.
    for (int n = 0, attempts = 0; n < counts.ambient && attempts < counts.ambient * 400; ++attempts) {
        const float e = CloudField::kExtent * 0.95f;
        const float x = rng.range(-e, e), y = rng.range(-e * 0.6f, e * 0.6f), z = rng.range(-e * 0.8f, e * 0.8f);
        const float r = std::sqrt(x * x + y * y + z * z);
        if (r < kCoreRadius * 0.85f) continue;
        const CloudComponents c = cloud.sample({x, y, z});
        const float p = std::min(c.envelope * 0.8f + c.filament, 1.0f);
        if (rng.uniform() > p * p * 1.2f) continue;
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
    for (int n = 0; n < counts.wind; ++n) push(out, 0, 0, 0, 4, rng);
    return out;
}

void ParticleSystem::init(const CloudField& cloud, const ParticleCounts& counts, uint32_t seed) {
    const std::vector<ParticleVertex> data = generateParticles(counts, seed, cloud);
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
