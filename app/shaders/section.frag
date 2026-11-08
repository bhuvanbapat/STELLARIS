#version 450
// Scientific cross-section: the plane through the dense core that faces the
// camera, colored by the NORMALIZED procedural density rho_n from the very same
// function the volume renderer integrates (cloud_density.glsl), at the current
// timeline time. Drawn after tone mapping so the color map is exact (viridis,
// perceptually ordered; the UI legend uses the same polynomial).
#include "common.glsl"
#include "cloud_density.glsl"

in vec2 vLocal;
out vec4 outColor;
uniform vec3 uCenter;
uniform float uSize;
uniform float uRangeMax;   // density mapped to the top of the color map

// Polynomial fit of matplotlib's viridis (sRGB output).
vec3 viridis(float t) {
    const vec3 c0 = vec3(0.2777273272234177, 0.005407344544966578, 0.3340998053353061);
    const vec3 c1 = vec3(0.1050930431085774, 1.404613529898575, 1.384590162594685);
    const vec3 c2 = vec3(-0.3308618287255563, 0.214847559468213, 0.09509516302823659);
    const vec3 c3 = vec3(-4.634230498983486, -5.799100973351585, -19.33244095627987);
    const vec3 c4 = vec3(6.228269936347081, 14.17993336680509, 56.69055260068105);
    const vec3 c5 = vec3(4.776384997670288, -13.74514537774601, -65.35303263337234);
    const vec3 c6 = vec3(-5.435455855934631, 4.645852612178535, 26.3124352495832);
    return c0 + t * (c1 + t * (c2 + t * (c3 + t * (c4 + t * (c5 + t * c6)))));
}

void main() {
    vec3 right = vec3(uView[0][0], uView[1][0], uView[2][0]);
    vec3 up    = vec3(uView[0][1], uView[1][1], uView[2][1]);
    vec3 p = uCenter + (right * vLocal.x + up * vLocal.y) * uSize;
    float d = density(p);
    if (d < 0.004) discard;  // empty space stays see-through
    float t = saturate(d / uRangeMax);
    // Thin iso-contours at quarter steps help read the gradient.
    float q = fract(t * 4.0);
    float contour = 1.0 - 0.25 * (1.0 - smoothstep(0.0, 0.04, q)) * step(0.02, t);
    float edge = 1.0 - smoothstep(0.92, 1.0, max(abs(vLocal.x), abs(vLocal.y)));
    outColor = vec4(viridis(t) * contour, 0.88 * edge);
}
