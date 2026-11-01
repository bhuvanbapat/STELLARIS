#pragma once
// Procedural meshes: a UV sphere, a polar-grid annulus (for the accretion disc)
// and an open cone (for pulsar beams). All are generated at startup.

#include "gfx/GL.h"

namespace stellaris {

struct Mesh {
    gl::VertexArray vao;
    gl::Buffer      vertices;
    gl::Buffer      indices;
    GLsizei         indexCount = 0;

    void draw() const;
};

// Unit sphere; attribute 0 = position (also the normal).
Mesh makeSphere(int slices, int stacks);
// Annulus in the XZ plane with inner radius 0 and outer radius 1, generated as
// a polar grid; attribute 0 = vec2(radius in [0,1], angle in radians).
Mesh makeAnnulusGrid(int radialSegments, int angularSegments);
// Open cone with apex at the origin, axis +Y, height 1, end radius 1;
// attribute 0 = vec3 position.
Mesh makeCone(int segments, int lengthSegments);

} // namespace stellaris
