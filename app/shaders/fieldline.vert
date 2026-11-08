#version 450
// One camera-facing quad per field-line segment (instanced), with a fixed width in
// pixels, so no wide hardware lines are needed.
#include "common.glsl"

layout(std430, binding = 3) readonly buffer Segments { vec4 seg[]; };  // pairs a, b; w = radius
uniform mat4 uModel;
uniform float uWidthPx;

out float vAcross;
out float vRadius;   // distance from the center in stellar radii

void main() {
    vec4 a = seg[2 * gl_InstanceID];
    vec4 b = seg[2 * gl_InstanceID + 1];
    vec4 ca = uViewProj * (uModel * vec4(a.xyz, 1.0));
    vec4 cb = uViewProj * (uModel * vec4(b.xyz, 1.0));
    vec2 sa = ca.xy / ca.w, sb = cb.xy / cb.w;
    vec2 dir = (sb - sa) * uViewport.xy;
    dir = length(dir) > 1e-6 ? normalize(dir) : vec2(1.0, 0.0);
    vec2 offset = vec2(-dir.y, dir.x) * uWidthPx * uViewport.zw;   // half-width in NDC

    bool atB = (gl_VertexID & 1) == 1;
    float side = (gl_VertexID & 2) == 0 ? -1.0 : 1.0;
    vec4 c = atB ? cb : ca;
    c.xy += offset * side * c.w;
    gl_Position = c;
    vAcross = side;
    vRadius = atB ? b.w : a.w;
}
