#version 450
// Soft radial halo around a luminous object, additive. x is the distance from
// the center in units of the object's radius; the halo starts at the limb.
#include "common.glsl"
in vec2 vLocal;
out vec4 outColor;
uniform vec3 uColor;
uniform float uIntensity;
uniform float uInnerFrac;  // object radius as a fraction of the quad half-size
uniform float uFalloff;    // larger = tighter halo
uniform int uRays;         // 1 = add faint streak structure (supernova flash)
void main() {
    float d = length(vLocal);
    if (d >= 1.0) discard;
    float x = max(d / uInnerFrac, 1.0);
    float halo = exp(-(x - 1.0) * uFalloff) * 0.7 + 0.3 / (x * x * x);
    halo *= smoothstep(1.0, 0.75, d);          // fade before the quad edge
    if (uRays == 1) {
        float a = atan(vLocal.y, vLocal.x);
        halo *= 0.75 + 0.25 * pow(abs(sin(a * 3.0)), 8.0) + 0.15 * pow(abs(sin(a * 7.0 + 1.0)), 16.0);
    }
    outColor = vec4(uColor * halo * uIntensity, 0.0);
}
