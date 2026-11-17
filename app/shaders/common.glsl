// Shared per-frame data. Must match FrameUniforms in render/Renderer.cpp (std140).
layout(std140, binding = 0) uniform Frame {
    mat4 uView;
    mat4 uProj;
    mat4 uViewProj;
    mat4 uInvViewProj;
    vec4 uCamPos;        // xyz camera position, w = timeline time (s)
    vec4 uViewport;      // width, height, 1/width, 1/height (of the current target)
    vec4 uCloud;         // cloudDensity, collapse, coreGlow, cavity
    vec4 uCloud2;        // dispersal, infallClock, rotation, discScale
    vec4 uCentral;       // presence, radius, intensity, turbulence
    vec4 uCentralColor;  // rgb, protostarVeil
    vec4 uDisc;          // presence, outerRadius, innerRadius, jetPresence
    vec4 uMisc;          // jetLength, giant, fateProgress, exposure
    vec4 uEjecta;        // presence, clock, kind, glow
    vec4 uEjecta2;       // strength, supernovaFlash, remnantPresence, remnantRadius
    vec4 uRemnantColor;  // rgb, intensity
    vec4 uBlackHole;     // presence, shadowRadius, einsteinRadius, pulsarBeams
    vec4 uWind;          // strength, speed, dusty, unused
    vec4 uInner;         // streamers, giantAtmosphere, remnantData (0/1), circumstellarRing
};

#define TIME (uCamPos.w)
const float PI = 3.14159265358979;

// Blackbody color lookup (linear sRGB, max channel = 1), built on the CPU from
// the same function the scientific model uses. Domain: log10(T) in [3, 4.7].
layout(binding = 7) uniform sampler1D uBlackbody;
vec3 blackbody(float kelvin) {
    float u = (log(max(kelvin, 1000.0)) / log(10.0) - 3.0) / 1.7;
    return texture(uBlackbody, clamp(u, 0.0, 1.0)).rgb;
}

// Integer hash (PCG-style); deterministic on every GPU.
uvec3 pcg3d(uvec3 v) {
    v = v * 1664525u + 1013904223u;
    v.x += v.y * v.z; v.y += v.z * v.x; v.z += v.x * v.y;
    v ^= v >> 16u;
    v.x += v.y * v.z; v.y += v.z * v.x; v.z += v.x * v.y;
    return v;
}
vec3 hash33(vec3 p) { return vec3(pcg3d(uvec3(ivec3(floor(p)) + 32768))) * (1.0 / 4294967295.0); }

float saturate(float x) { return clamp(x, 0.0, 1.0); }

// Supernova remnant geometry shared by the volume shell and the ejecta particles.
// Shock radius: fast early expansion that decelerates (presentation timing).
float snShockRadius(float clock, float strength) {
    return 0.6 + 24.0 * mix(0.75, 1.0, strength) * (1.0 - exp(-clock / 9.0));
}
// Fixed large-scale asymmetry: a dipole (one side expands further) plus a
// quadrupole. Illustrative: real remnants are asymmetric, these axes are arbitrary.
float snAsymmetry(vec3 dir) {
    float dip = dot(dir, normalize(vec3(1.0, 0.15, 0.2)));
    float quad = dot(dir, normalize(vec3(-0.2, 1.0, 0.3)));
    return 1.0 + 0.16 * dip + 0.08 * (1.5 * quad * quad - 0.5);
}
float remap(float v, float lo, float hi, float nlo, float nhi) {
    return nlo + (v - lo) * (nhi - nlo) / (hi - lo);
}
