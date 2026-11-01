#version 450
// Accretion disc surface: a polar grid, drawn as several thin instanced layers
// offset vertically by the local scale height (flared disc, h ~ r^1.25).
#include "common.glsl"
layout(location = 0) in vec2 aPolar;   // (radius in [0,1], angle)
out vec3 vWorld;
out float vRadius;
out float vAngle;
flat out int vLayer;
uniform int uLayers;
void main() {
    float inner = uDisc.z, outer = max(uDisc.y, inner + 0.1);
    float r = inner + (outer - inner) * pow(aPolar.x, 1.35);
    float h = 0.045 * pow(r, 1.25);
    float layer = (uLayers > 1) ? (float(gl_InstanceID) / float(uLayers - 1) * 2.0 - 1.0) : 0.0;
    vec3 p = vec3(r * cos(aPolar.y), layer * h * 1.4, r * sin(aPolar.y));
    vWorld = p;
    vRadius = r;
    vAngle = aPolar.y;
    vLayer = gl_InstanceID;
    gl_Position = uViewProj * vec4(p, 1.0);
}
