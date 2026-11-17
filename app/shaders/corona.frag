#version 450
// Outer atmosphere of the main-sequence star, drawn on a camera-facing quad with
// depth testing (the stellar sphere hides the part of the quad behind its disc):
//   chromosphere - a thin reddish emission rim right at the limb;
//   corona       - low-density outer emission with radial streamers, strongest
//                  toward the equator, asymmetric and slowly varying.
// Streamer directions are tied to world-space directions (not the screen), so
// they stay put as the camera orbits. Illustrative, not a coronal model.
#include "common.glsl"
#include "stellar_activity.glsl"

in vec2 vLocal;
out vec4 outColor;
layout(binding = 0) uniform sampler3D uNoise;
uniform vec3 uCenter;
uniform float uSize;         // quad half-size (world units)
uniform float uRadius;       // stellar radius (world units)
uniform vec3 uColor;         // photospheric color
uniform float uIntensity;
uniform float uActivity;     // 0: smooth hot-star halo only; 1: full structure
uniform float uFlare;

void main() {
    float d = length(vLocal);
    if (d >= 1.0) discard;
    float b = d * uSize / uRadius;             // impact parameter in stellar radii
    if (b < 0.98) discard;                      // the disc itself is the photosphere

    // World-space direction of this point as seen from the star's center.
    vec3 right = vec3(uView[0][0], uView[1][0], uView[2][0]);
    vec3 up    = vec3(uView[0][1], uView[1][1], uView[2][1]);
    vec3 dir = normalize(right * vLocal.x + up * vLocal.y);
    vec3 sdir = stellarFrame(dir, TIME);

    float edgeFade = 1.0 - smoothstep(0.7, 1.0, d);
    float h = max(b - 1.0, 0.0);               // height above the limb (radii)

    // Smooth, featureless halo (all stars; the only layer for hot stars).
    float halo = (0.55 * exp(-h * 3.0) + 0.12 / (b * b * b)) * edgeFade;

    // Streamers: angular structure from low-frequency noise on the direction,
    // concentrated toward the stellar equator, extending radially.
    float n1 = texture(uNoise, sdir * 0.45 + vec3(0.37, TIME * 0.0015, 0.0)).r;
    float n2 = texture(uNoise, sdir * 1.30 + vec3(0.0, 0.11, TIME * 0.002)).g;
    float streak = pow(saturate(n1 * 1.4 - 0.25), 3.0) * 2.4 + pow(saturate(n2 * 1.3 - 0.35), 4.0);
    float equator = pow(1.0 - abs(sdir.y), 1.5);
    float corona = streak * equator * exp(-h * 0.9) * (0.35 + 0.65 * exp(-h * 2.5));

    // Coronal emission concentrates above active regions (as in SDO/AIA EUV images):
    // brightness from the real magnetogram, blurred, at the limb position.
    if (uHasMagnetogram != 0) {
        float act = smoothstep(20.0, 160.0, magneticField(sdir, 5.0));
        float loops = 0.6 + 0.4 * sin(texture(uNoise, sdir * 6.0).b * 30.0);
        corona += act * loops * exp(-h * 3.2) * 1.6;
    }
    // Chromosphere: thin rim just above the limb (hydrogen-alpha-like red), made
    // ragged by spicules, the jets of chromospheric gas seen at the solar limb.
    float spic = texture(uNoise, sdir * 38.0 + vec3(0.0, TIME * 0.05, 0.0)).r;
    float chromo = exp(-pow(max(h - 0.012 * spic * spic * 2.0, 0.0) / 0.018, 2.0));

    // Flare: brief brightening of the corona above the flare site.
    float flareGlow = exp(-pow(acos(clamp(dot(sdir, flareCenter()), -1.0, 1.0)) / 0.25, 2.0)) * exp(-h * 2.0) * uFlare;

    vec3 coronaColor = mix(uColor, vec3(0.85, 0.9, 1.0), 0.4);
    vec3 col = uColor * halo * mix(1.0, 0.45, uActivity)
             + coronaColor * corona * uActivity * 0.9
             + vec3(1.0, 0.32, 0.38) * chromo * uActivity * 1.4
             + vec3(1.0, 0.95, 0.9) * flareGlow * 2.0;
    outColor = vec4(col * uIntensity * edgeFade, 0.0);
}
