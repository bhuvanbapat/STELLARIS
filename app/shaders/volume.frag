#version 450
// Ray-marched volumetric molecular cloud (rendered at reduced resolution).
//
// Density: seeded structure field (envelope, filaments, core, globules; see
//   sim/CloudField) eroded by 3D noise detail.
// Collapse: the density at a point is read from a larger "source" radius
//   (r0 = r * h(r)), so material appears drawn inward, inside-out, with a
//   Jacobian brightening of the center, a swirl (angular momentum) and a
//   flattening toward the equatorial plane. This is a prescribed visual
//   mapping, not a hydrodynamic solution.
// Lighting: single scattering from (1) a distant external radiation field and
//   (2) the central protostar/star, each with a short shadow march, a
//   Henyey-Greenstein phase function and wavelength-dependent dust extinction.
// Output: premultiplied radiance (rgb) and opacity (a = 1 - transmittance).
#include "common.glsl"

in vec2 vUV;
out vec4 outColor;
layout(binding = 0) uniform sampler3D uNoise;
layout(binding = 1) uniform sampler2D uSceneDepth;
uniform int uSteps;

uniform vec3 uKeyDir;  // toward the external light (CloudField::keyLightDirection: behind the cloud)
#define KEY_DIR uKeyDir
const vec3  EXTINCTION_TINT = vec3(0.80, 1.0, 1.22);       // dust extinguishes blue more

vec4 noiseAt(vec3 p) { return texture(uNoise, p); }

// Large-scale structure baked on the CPU from a seed (sim/CloudField):
// r = envelope, g = filament network, b = dense core + condensations, a = globules.
// Normalized, dimensionless procedural densities.
layout(binding = 2) uniform sampler3D uCloudStructure;
uniform float uCloudExtent;
vec4 structureAt(vec3 p) { return texture(uCloudStructure, p / (2.0 * uCloudExtent) + 0.5); }

// Undisturbed cloud density at position p (before collapse mapping). Each
// component receives its own treatment of the shared noise detail: the envelope
// is strongly eroded into wisps, filaments keep their ridge but gain texture,
// the core and globules stay compact.
float baseDensity(vec3 p, float drift) {
    vec3 warp = (noiseAt(p * 0.012 + vec3(0.0, 0.0, drift)).gba - 0.5) * 5.0;
    vec3 q = p + warp;
    vec4 s = structureAt(q);
    if (s.r + s.g + s.b + s.a < 0.015) return 0.0;
    vec4 n1 = noiseAt(q * 0.016 + vec3(drift * 0.5, 0.0, 0.0));
    vec4 n2 = noiseAt(q * 0.052 - vec3(0.0, drift, 0.0));
    vec4 n3 = noiseAt(q * 0.13 + vec3(0.0, 0.0, drift));
    float detail = n1.r * 0.58 + n2.g * 0.26 + n2.a * 0.16 + (n3.a - 0.5) * 0.16;
    float env = saturate(remap(detail, 1.0 - s.r * 0.85, 1.0, 0.0, 1.0));
    env = env * env * 1.6;
    float fil = s.g * saturate(0.35 + 1.1 * detail) * 1.5;
    float core = s.b * (0.75 + 0.5 * detail) * 1.4;
    float glob = s.a * 3.0;
    return env + fil + core + glob;
}

// Collapse mapping and outflow carving, shared by the full and the coarse density.
// q: where in the undisturbed cloud the material now at p came from;
// gain: Jacobian density increase from contraction; mask: clearing/cavity/feedback.
struct Mapped { vec3 q; float gain; float mask; };
Mapped collapseMap(vec3 p) {
    float collapse = uCloud.y;
    float r = length(p);
    float A = 4.0 * collapse;
    float L = 7.0;
    float e = exp(-r / L);
    float h = 1.0 + A * e;
    float dr0dr = 1.0 + A * e * (1.0 - r / L);
    float jac = h * h * dr0dr;

    // Swirl and flattening grow toward the center as the core contracts.
    float swirl = collapse * uCloud2.z * 2.2 * exp(-r / 9.0);
    float cs = cos(swirl), sn = sin(swirl);
    vec3 q = vec3(cs * p.x - sn * p.z, p.y, sn * p.x + cs * p.z);
    q.y *= 1.0 + collapse * 1.8 * exp(-r / 11.0);
    q *= h;

    // Inner envelope is accreted / cleared as the disc and star form.
    float clear = uCloud.w;
    float mask = mix(1.0, smoothstep(2.0, 14.0, r), clear * 0.85);
    // Bipolar outflow cavities along the rotation axis.
    float cosAxis = abs(p.y) / max(r, 1e-3);
    float coneCos = cos(radians(mix(8.0, 34.0, clear)));
    mask *= mix(1.0, 0.06, clear * smoothstep(coneCos - 0.06, coneCos + 0.04, cosAxis) * smoothstep(1.0, 4.0, r));
    // Feedback from the star blows the envelope outward and thins it.
    mask *= 1.0 - uCloud2.x * 0.93 * exp(-r / 28.0);

    return Mapped(q, min(sqrt(jac), 4.0), mask * uCloud.x);
}

// Full density (structure + noise detail) for the view ray.
float density(vec3 p) {
    Mapped m = collapseMap(p);
    if (m.mask <= 0.0) return 0.0;
    return baseDensity(m.q, TIME * 0.0012) * m.gain * m.mask;
}

// Coarse density for shadow rays: the structure field with the noise detail
// replaced by its typical contribution. Shadows are soft, so the missing detail is
// barely visible there; this is a documented rendering approximation.
float densityCoarse(vec3 p) {
    Mapped m = collapseMap(p);
    if (m.mask <= 0.0) return 0.0;
    vec4 s = structureAt(m.q);
    return (s.r * s.r * 0.55 + s.g * 0.9 + s.b * 1.4 + s.a * 3.0) * m.gain * m.mask;
}

// True when the undisturbed structure is empty near p (used to take larger steps).
bool emptyAt(vec3 p) {
    vec4 s = structureAt(collapseMap(p).q);
    return s.r + s.g + s.b + s.a < 0.01;
}

// Emission from final-stage ejecta: a thin, clumpy shell whose radius follows
// the same expansion law as the ejecta particles (artistic colors: ionized
// oxygen teal inside, hydrogen-alpha red outside).
vec3 shellEmission(vec3 p) {
    float presence = uEjecta.x;
    if (presence <= 0.0) return vec3(0.0);
    float clock = uEjecta.y;
    vec4 n = noiseAt(p * 0.045 + vec3(0.3, clock * 0.002, 0.0));
    vec4 n2 = noiseAt(p * 0.11);
    float clumps = smoothstep(0.35, 0.85, n.r * 0.7 + n2.a * 0.45);
    if (uEjecta.z < 0.5) {
        // Bipolar planetary nebula: faster expansion along the axis.
        float dirY = abs(p.y) / max(length(p), 1e-3);
        float stretch = 1.0 + 0.6 * dirY * dirY;
        float R = 4.2 + 0.29 * clock;
        float re = length(p) / stretch;
        float rel = re / R;
        float shell = exp(-pow((rel - 0.92) / 0.10, 2.0)) + 0.35 * exp(-pow((rel - 0.62) / 0.16, 2.0));
        vec3 col = mix(vec3(0.15, 0.85, 0.75), vec3(1.0, 0.28, 0.22), smoothstep(0.7, 1.0, rel));
        return col * shell * clumps * presence * uEjecta.w * 0.018;
    }
    float strength = uEjecta2.x;
    float R = 0.6 + 30.0 * 0.75 * mix(0.75, 1.0, strength) * (1.0 - exp(-clock / 9.0));
    float rel = length(p) / max(R, 0.5);
    float shell = exp(-pow((rel - 0.95) / 0.09, 2.0));
    vec3 col = mix(vec3(1.0, 0.30, 0.22), vec3(0.30, 0.80, 1.0), smoothstep(0.45, 0.65, n2.b));
    float hot = exp(-clock / 2.5);
    col = mix(col, vec3(1.0, 0.95, 0.9) * 3.0, hot);
    return col * shell * clumps * presence * strength * 0.016;
}

// True when p is close enough to the analytic ejecta shell to need full sampling
// (mirrors the shell radii used in shellEmission).
bool nearShell(vec3 p) {
    if (uEjecta.x <= 0.0) return false;
    float clock = uEjecta.y;
    if (uEjecta.z < 0.5) {
        float dirY = abs(p.y) / max(length(p), 1e-3);
        float rel = (length(p) / (1.0 + 0.6 * dirY * dirY)) / (4.2 + 0.29 * clock);
        return rel > 0.35 && rel < 1.3;
    }
    float R = 0.6 + 30.0 * 0.75 * mix(0.75, 1.0, uEjecta2.x) * (1.0 - exp(-clock / 9.0));
    float rel = length(p) / max(R, 0.5);
    return rel > 0.7 && rel < 1.25;
}

float phaseHG(float cosTheta, float g) {
    float g2 = g * g;
    return (1.0 - g2) / (4.0 * PI * pow(1.0 + g2 - 2.0 * g * cosTheta, 1.5));
}

// Optical depth along a short segment (4 samples).
float opticalDepth(vec3 from, vec3 dir, float len) {
    float tau = 0.0;
    const float f[4] = float[](0.08, 0.22, 0.45, 0.80);
    for (int i = 0; i < 4; ++i) tau += densityCoarse(from + dir * (f[i] * len));
    return tau * len * 0.25;
}

vec2 sphereHit(vec3 ro, vec3 rd, float R) {
    float b = dot(ro, rd);
    float c = dot(ro, ro) - R * R;
    float disc = b * b - c;
    if (disc < 0.0) return vec2(1.0, -1.0);
    float s = sqrt(disc);
    return vec2(-b - s, -b + s);
}

float ign(vec2 px) { return fract(52.9829189 * fract(dot(px, vec2(0.06711056, 0.00583715)))); }

void main() {
    vec2 ndc = vUV * 2.0 - 1.0;
    vec4 farP = uInvViewProj * vec4(ndc, 1.0, 1.0);
    vec3 ro = uCamPos.xyz;
    vec3 rd = normalize(farP.xyz / farP.w - ro);

    vec2 hit = sphereHit(ro, rd, uCloudExtent);
    float tNear = max(hit.x, 0.0), tFar = hit.y;

    // Stop at opaque geometry (the star) using the full-resolution depth buffer.
    float depth = texture(uSceneDepth, vUV).r;
    if (depth < 1.0) {
        vec4 wp = uInvViewProj * vec4(ndc, depth * 2.0 - 1.0, 1.0);
        tFar = min(tFar, length(wp.xyz / wp.w - ro));
    }
    if (tFar <= tNear || (uCloud.x <= 0.001 && uEjecta.x <= 0.0)) { outColor = vec4(0.0); return; }

    // Fixed step length: the sample pattern must not depend on where the ray
    // ends (otherwise the star's silhouette shows up as banding in the cloud).
    float stepLen = 2.0 * uCloudExtent / float(uSteps);
    float t = tNear + stepLen * ign(gl_FragCoord.xy);

    const float SIGMA = 0.26;  // extinction per unit density per scene unit
    vec3 transmittance = vec3(1.0);
    vec3 radiance = vec3(0.0);

    vec3 keyColor = vec3(0.62, 0.74, 1.0) * 0.9;                  // external field
    vec3 centralColor = uCentralColor.rgb;
    float centralPower = uCentral.x * uCentral.z * 1.1 + uCloud.z * 1.5;
    vec3 glowColor = mix(vec3(1.0, 0.42, 0.18), centralColor, uCentral.x);

    float cosKey = dot(rd, KEY_DIR);
    float phaseKey = phaseHG(cosKey, 0.35) * 4.0 * PI * 0.5 + 0.5;

    // Empty-space skipping: where the structure field is empty, advance two steps
    // at once (still bounded by uSteps iterations). Inside the cloud the step stays
    // fixed, so the sample pattern does not depend on where the ray ends.
    for (int i = 0; i < uSteps && t < tFar; ++i) {
        vec3 p = ro + rd * t;
        if (emptyAt(p) && !nearShell(p)) { t += 2.0 * stepLen; continue; }
        float d = density(p);
        radiance += transmittance * shellEmission(p) * stepLen;
        if (d > 0.002) {
            vec3 sigmaT = SIGMA * d * EXTINCTION_TINT;

            // External light.
            float tauKey = opticalDepth(p, KEY_DIR, 22.0) * SIGMA;
            vec3 lightKey = keyColor * exp(-tauKey * EXTINCTION_TINT * 3.0) * phaseKey;

            // Central source (inverse-square with softening), shadowed by the
            // gas between the point and the center.
            float r = length(p);
            vec3 lightC = vec3(0.0);
            if (centralPower > 0.005) {  // no shadow march while the central source is off
                vec3 toC = -p / max(r, 1e-3);
                float tauC = opticalDepth(p, toC, r * 0.92) * SIGMA;
                float fall = centralPower / (1.0 + r * r * 0.035);
                float phaseC = phaseHG(dot(rd, toC), 0.45) * 4.0 * PI * 0.6 + 0.4;
                lightC = glowColor * fall * exp(-tauC * EXTINCTION_TINT * 1.6) * phaseC;
            }

            // Faint ambient sky light keeps shadowed regions from going pure black.
            vec3 ambient = vec3(0.020, 0.024, 0.055) * (0.6 + 0.4 * saturate(p.y / 30.0 + 0.5));
            // Subtle dusty-magenta emission in dense, unshadowed filaments (artistic).
            vec3 emission = vec3(0.30, 0.08, 0.26) * 0.10 * smoothstep(0.25, 0.9, d) * exp(-tauKey * 0.6);

            vec3 albedo = vec3(0.62, 0.70, 0.92);
            vec3 S = (albedo * (lightKey + lightC + ambient) + emission) * sigmaT;
            vec3 Tstep = exp(-sigmaT * stepLen);
            // Energy-conserving integration of in-scattering over the step.
            radiance += transmittance * (S - S * Tstep) / max(sigmaT, vec3(1e-5));
            transmittance *= Tstep;
            if (max(transmittance.r, max(transmittance.g, transmittance.b)) < 0.01) break;
        }
        t += stepLen;
    }

    float alpha = 1.0 - dot(transmittance, vec3(0.3333));
    outColor = vec4(radiance, alpha);
}
