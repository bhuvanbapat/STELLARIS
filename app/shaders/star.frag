#version 450
// Stellar photosphere: blackbody base color, linear limb darkening, a multi-scale
// granulation-like pattern, starspots and a flare footpoint. Also used for compact
// remnants (no pattern, no activity).
//
// Everything here is a parametric visual representation (SCIENTIFIC_MODEL §20.4):
// no convection, magnetic flux emergence or reconnection is simulated. Spots and
// the flare only appear on stars with an outer convection zone (uActivity > 0).
#include "common.glsl"
#include "stellar_activity.glsl"

in vec3 vNormal;
in vec3 vWorld;
out vec4 outColor;
layout(binding = 0) uniform sampler3D uNoise;

uniform vec3  uColor;
uniform float uIntensity;
uniform float uTurbulence;    // 0 = smooth (hot star), 1 = large high-contrast cells (giants)
uniform float uVeil;          // protostar still wrapped in its envelope
uniform float uLimbU;         // linear limb-darkening coefficient
uniform float uSeed;
uniform float uActivity;      // 0..1: spots/flare visibility (convective, main-sequence star)
uniform float uFlare;         // 0..1 flare pulse
uniform float uPixelsPerRadius;  // projected size, used to fade sub-pixel detail

void main() {
    vec3 n = normalize(vNormal);
    vec3 v = normalize(uCamPos.xyz - vWorld);
    float mu = saturate(dot(n, v));

    // Linear limb-darkening law: I(mu) = 1 - u (1 - mu).
    float limb = 1.0 - uLimbU * (1.0 - mu);

    // Surface coordinates rotate with the star (deterministic in time).
    vec3 sp = stellarFrame(n, TIME);

    // Granulation-like pattern at two scales. Cells smaller than a few pixels fade
    // to their mean instead of aliasing; giants get few, large, contrasty cells.
    float scale = mix(0.55, 0.16, uTurbulence);
    float t = TIME;
    vec4 a = texture(uNoise, sp * scale + vec3(uSeed, t * 0.004, 0.0));
    vec4 b = texture(uNoise, sp * scale * 2.7 + vec3(0.0, -t * 0.006, uSeed));
    float cellPixels = uPixelsPerRadius / max(scale * 8.0, 1e-3);   // rough projected cell size
    float detailFade = smoothstep(2.0, 6.0, cellPixels);
    float cells = mix(0.55, a.g * 0.65 + b.g * 0.35, detailFade);
    float contrast = mix(0.12, 1.1, uTurbulence);
    float pattern = clamp(1.0 + (cells - 0.55) * contrast * 2.0, 0.25, 1.6);
    // Cooler, darker lanes are slightly redder.
    vec3 lane = mix(vec3(1.0, 0.55, 0.35), vec3(1.0), smoothstep(0.5, 1.1, pattern));

    // Starspots: cooler regions (dark umbra, lighter penumbra) with ragged edges.
    float spot = spotDarkening(sp, uNoise) * uActivity;
    vec3 spotTint = mix(vec3(1.0), vec3(1.0, 0.72, 0.5), spot);
    float spotDim = 1.0 - 0.8 * spot;

    vec3 col = uColor * lane * pattern * limb * spotTint * spotDim * uIntensity;

    // Flare footpoint: a localized, short-lived brightening beside the main spot.
    float flare = flareKernel(sp) * uFlare;
    col += vec3(1.0, 0.92, 0.85) * flare * uIntensity * 3.0;

    // A young protostar is seen through its dusty envelope: dimmer and redder.
    vec3 veiled = vec3(1.0, 0.35, 0.12) * uIntensity * 0.45 * (0.6 + 0.4 * pattern) * limb;
    col = mix(col, veiled, uVeil);

    outColor = vec4(col, 1.0);
}
