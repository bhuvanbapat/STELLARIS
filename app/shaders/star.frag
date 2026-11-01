#version 450
// Stellar surface: blackbody base color, linear limb darkening and an animated
// cellular pattern. The pattern is an artistic stand-in for granulation; it is
// not a convection simulation. Also used for compact remnants (no pattern).
#include "common.glsl"

in vec3 vNormal;
in vec3 vWorld;
out vec4 outColor;
layout(binding = 0) uniform sampler3D uNoise;

uniform vec3  uColor;
uniform float uIntensity;
uniform float uTurbulence;   // 0 = smooth, 1 = large high-contrast cells (giants)
uniform float uVeil;         // protostar still wrapped in its envelope
uniform float uLimbU;        // linear limb-darkening coefficient
uniform float uSeed;

void main() {
    vec3 n = normalize(vNormal);
    vec3 v = normalize(uCamPos.xyz - vWorld);
    float mu = saturate(dot(n, v));

    // Linear limb-darkening law: I(mu) = 1 - u (1 - mu).
    float limb = 1.0 - uLimbU * (1.0 - mu);

    // Rotating cellular pattern sampled on the sphere; two scales.
    float t = TIME;
    float rot = t * 0.02;
    vec3 sp = vec3(cos(rot) * n.x - sin(rot) * n.z, n.y, sin(rot) * n.x + cos(rot) * n.z);
    float scale = mix(0.55, 0.16, uTurbulence);
    vec4 a = texture(uNoise, sp * scale + vec3(uSeed, t * 0.004, 0.0));
    vec4 b = texture(uNoise, sp * scale * 2.7 + vec3(0.0, -t * 0.006, uSeed));
    float cells = a.g * 0.65 + b.g * 0.35;
    float contrast = mix(0.35, 1.1, uTurbulence);
    float pattern = clamp(1.0 + (cells - 0.55) * contrast * 2.0, 0.25, 1.6);
    // Cooler, darker lanes are slightly redder.
    vec3 lane = mix(vec3(1.0, 0.55, 0.35), vec3(1.0), smoothstep(0.5, 1.1, pattern));

    vec3 col = uColor * lane * pattern * limb * uIntensity;

    // A young protostar is seen through its dusty envelope: dimmer and redder.
    vec3 veiled = vec3(1.0, 0.35, 0.12) * uIntensity * 0.45 * (0.6 + 0.4 * pattern) * limb;
    col = mix(col, veiled, uVeil);

    outColor = vec4(col, 1.0);
}
