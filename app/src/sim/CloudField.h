#pragma once
// Procedural, seeded large-scale structure of the molecular cloud.
//
// The cloud is built from separately identifiable components so each one can be
// inspected, tested and tuned:
//   envelope  - broad irregular cloud body with low-density voids
//   filament  - a hub-filament network converging on the dense core, with branches
//   core      - the dense core at the origin plus secondary condensations
//   globule   - compact, very dense globules (dark silhouettes, bright rims)
// All values are NORMALIZED, dimensionless visual densities: a procedural density
// structure, not a calculated turbulent-cloud solution (SCIENTIFIC_MODEL.md §20.1).
//
// The field is evaluated on the CPU (deterministic on every machine), baked into a
// voxel grid, uploaded as a 3D texture for the volume renderer, and sampled by the
// particle generator, so every consumer sees the same structure.

#include <glm/vec3.hpp>

#include <cstdint>
#include <vector>

namespace stellaris {

struct CloudComponents {
    float envelope = 0.0f;
    float filament = 0.0f;
    float core     = 0.0f;
    float globule  = 0.0f;
};

class CloudField {
public:
    static constexpr float    kExtent = 56.0f;   // field covers [-kExtent, kExtent]^3 (scene units)
    static constexpr uint32_t kDefaultSeed = 1;
    // Normalized density mapped to the top of the scientific cross-section color map.
    static constexpr float    kSectionRangeMax = 4.0f;
    // Unit vector toward the external illuminating source (behind the cloud, upper
    // left). The volume renderer lights with it; globule tails point away from it.
    static glm::vec3 keyLightDirection();

    // Builds the geometry for `seed` and bakes a resolution^3 grid (multi-threaded).
    static CloudField build(uint32_t seed, int resolution = 128);

    // Exact procedural evaluation (no grid), used for baking and tests.
    CloudComponents evaluate(const glm::vec3& p) const;
    // Trilinear lookup in the baked grid (what the renderer sees).
    CloudComponents sample(const glm::vec3& p) const;

    uint32_t seed() const { return seed_; }
    int resolution() const { return resolution_; }
    // Interleaved RGBA = (envelope, filament, core, globule), x fastest, then y, then z.
    const std::vector<float>& voxels() const { return voxels_; }

    // Points along the centerline of filament i (for tests / inspection).
    int filamentCount() const { return static_cast<int>(filaments_.size()); }
    const std::vector<glm::vec3>& filamentPath(int i) const { return filaments_[i].points; }

private:
    struct Blob     { glm::vec3 center; glm::vec3 inverseRadii; float strength; };
    struct Filament { std::vector<glm::vec3> points; std::vector<float> widths; glm::vec3 boundsMin, boundsMax; float strength; };
    struct Clump    { glm::vec3 center; float radius; float strength; };

    uint32_t seed_ = kDefaultSeed;
    int resolution_ = 0;
    std::vector<Blob> envelopeBlobs_;
    std::vector<Filament> filaments_;
    std::vector<Clump> condensations_;
    std::vector<Clump> globules_;
    glm::vec3 noiseOffset_{0.0f};
    std::vector<float> voxels_;
};

// Small seeded value noise (CPU), trilinear with smooth interpolation, in [0, 1].
float valueNoise3(const glm::vec3& p, uint32_t seed);

} // namespace stellaris
