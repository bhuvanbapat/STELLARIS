#version 450
// Camera-facing quad centered on a world point (halo / corona / flash).
#include "common.glsl"
uniform vec3 uCenter;
uniform float uSize;   // half-size in world units
out vec2 vLocal;
void main() {
    vec2 corner = vec2((gl_VertexID & 1) * 2 - 1, ((gl_VertexID >> 1) & 1) * 2 - 1);
    vLocal = corner;
    vec3 right = vec3(uView[0][0], uView[1][0], uView[2][0]);
    vec3 up    = vec3(uView[0][1], uView[1][1], uView[2][1]);
    vec3 world = uCenter + (right * corner.x + up * corner.y) * uSize;
    gl_Position = uViewProj * vec4(world, 1.0);
}
