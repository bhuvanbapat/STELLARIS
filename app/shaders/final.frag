#version 450
// Final composite: exposure, bloom, ACES filmic tone mapping (Narkowicz fit),
// gentle vignette, sRGB encoding and dithering to avoid banding in dark space.
#include "common.glsl"
in vec2 vUV;
out vec4 outColor;
layout(binding = 0) uniform sampler2D uScene;
layout(binding = 1) uniform sampler2D uBloom;
uniform float uBloomStrength;

vec3 acesFitted(vec3 x) {
    return clamp((x * (2.51 * x + 0.03)) / (x * (2.43 * x + 0.59) + 0.14), 0.0, 1.0);
}
vec3 linearToSrgb(vec3 c) {
    return mix(c * 12.92, 1.055 * pow(c, vec3(1.0 / 2.4)) - 0.055, step(0.0031308, c));
}

void main() {
    vec3 scene = texture(uScene, vUV).rgb;
    vec3 bloom = texture(uBloom, vUV).rgb;
    vec3 hdr = mix(scene, bloom, uBloomStrength) * uMisc.w;   // uMisc.w = exposure

    vec3 col = acesFitted(hdr);
    vec2 q = vUV - 0.5;
    col *= 1.0 - 0.32 * dot(q, q) * 2.0;
    col = linearToSrgb(col);

    // Triangular dither (+/- 1/255) keyed to pixel position only.
    vec2 px = gl_FragCoord.xy;
    float n = fract(sin(dot(px, vec2(12.9898, 78.233))) * 43758.5453) +
              fract(sin(dot(px, vec2(39.3468, 11.135))) * 24634.6345) - 1.0;
    col += n / 255.0;
    outColor = vec4(col, 1.0);
}
