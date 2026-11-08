#pragma once
// GPU storage and drawing of precomputed analytic dipole field lines
// (see sim/MagneticField). Lines are generated once in a unit-radius frame with
// the dipole along +Y; a model matrix places, scales and orients them per frame.

#include "gfx/GL.h"

#include <glm/mat4x4.hpp>

namespace stellaris {

class FieldLines {
public:
    enum class Set { Star, NeutronStar };

    void init();
    // Draws one set as instanced camera-facing segment quads (shader bound by caller).
    void draw(Set set) const;
    int segmentCount(Set set) const { return set == Set::Star ? starSegments_ : nsSegments_; }

private:
    gl::Buffer starBuffer_, nsBuffer_;  // std430 vec4 pairs (a, b); w = |position| in radii
    gl::VertexArray vao_;
    int starSegments_ = 0, nsSegments_ = 0;
};

} // namespace stellaris
