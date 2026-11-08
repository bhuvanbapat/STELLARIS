#pragma once
// Static particle attributes (initial position, population, random seeds),
// generated once with a fixed seed. All motion is computed on the GPU from
// these attributes and the timeline time (see shaders/particles.vert).

#include "gfx/GL.h"
#include "sim/CloudField.h"

#include <cstdint>
#include <vector>

namespace stellaris {

struct ParticleVertex {
    float pos[4];   // xyz initial position, w population id
    float seed[4];  // uniform random numbers in [0, 1)
};

struct ParticleCounts {
    int ambient = 90000;
    int core    = 110000;
    int jet     = 16000;
    int ejecta  = 60000;
    int wind    = 14000;
    int total() const { return ambient + core + jet + ejecta + wind; }
};

// Pure CPU generation (unit-testable): same seed -> identical output.
// Ambient gas follows the envelope and filaments of `cloud`.
std::vector<ParticleVertex> generateParticles(const ParticleCounts& counts, uint32_t seed, const CloudField& cloud);

class ParticleSystem {
public:
    // (Re)builds the particle buffer; called again when the cloud seed changes.
    void init(const CloudField& cloud, const ParticleCounts& counts = {}, uint32_t seed = 20261102u);
    void draw() const;
    int count() const { return count_; }

private:
    gl::VertexArray vao_;
    gl::Buffer      vbo_;
    int             count_ = 0;
};

} // namespace stellaris
