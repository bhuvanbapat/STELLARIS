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
layout(binding = 1) uniform sampler2D uSceneDepth;
uniform int uSteps;
#include "cloud_density.glsl"

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
    // Supernova: a thin shock front (sharp outer edge, softer inner edge) with
    // filamentary brightness, plus faint emission from the ejecta inside it.
    float strength = uEjecta2.x;
    float r = length(p);
    vec3 dir = p / max(r, 1e-3);
    float R = snShockRadius(clock, strength) * snAsymmetry(dir);
    float rel = r / max(R, 0.5);
    float front = smoothstep(0.91, 0.99, rel) * (1.0 - smoothstep(0.99, 1.015, rel));
    // Patchy, filamentary rim: a ridged mask at two scales on the shell direction.
    vec4 nf = noiseAt(dir * 0.9 + vec3(0.21, 0.0, clock * 0.0008));
    vec4 nf2 = noiseAt(dir * 2.3 + vec3(0.0, 0.47, 0.13));
    // Threshold the fBm channel (centred near 0.5) across its middle so the rim breaks
    // into separate bright patches and dark gaps; edge-on limb brightening would
    // otherwise smooth mild variation into a uniform bubble. (The ridged channel was
    // tried first: its values sit mostly above any useful threshold.)
    float filaments = 0.02 + 1.3 * smoothstep(0.53, 0.62, nf.r) * (0.3 + 1.2 * smoothstep(0.45, 0.6, nf2.r));
    float interior = 0.22 * smoothstep(0.98, 0.45, rel) * smoothstep(0.1, 0.4, rel) * clumps;
    vec3 shockCol = vec3(0.55, 0.75, 1.0);
    vec3 ejectaCol = mix(vec3(1.0, 0.30, 0.22), vec3(0.30, 0.85, 0.65), smoothstep(0.45, 0.65, n2.b));
    float hot = exp(-clock / 2.0);
    float fade = 0.5 + 0.5 * exp(-clock / 25.0);
    vec3 col = (shockCol * front * filaments + ejectaCol * interior) * fade;
    col = mix(col, vec3(1.0, 0.95, 0.9) * front * 2.0, hot);
    return col * presence * strength * 0.014;
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
    float R = snShockRadius(clock, uEjecta2.x);
    float rel = length(p) / max(R, 0.5);
    return rel > 0.05 && rel < 1.3;
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

    const float SIGMA = 0.32;  // extinction per unit density per scene unit
    vec3 transmittance = vec3(1.0);
    vec3 radiance = vec3(0.0);

    vec3 keyColor = vec3(0.80, 0.86, 1.0) * 0.85;                 // external OB-star field
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
            // Photo-ionised skin: surfaces facing the external stars glow in H-alpha
            // (rose-red), as on the rims of the Pillars of Creation. Only the thin
            // unshadowed layer emits; the interior stays dark dust.
            float skin = exp(-tauKey * 3.0) * smoothstep(0.05, 0.35, d);
            vec3 emission = vec3(0.95, 0.30, 0.36) * 0.55 * skin;
            // Thin, ionised envelope gas: faint [O III] teal, as in HII regions.
            emission += vec3(0.20, 0.55, 0.58) * 0.10 * (1.0 - smoothstep(0.03, 0.25, d));
            // Compressional heating: the densest gas in the contracting core glows
            // from within, deep red at the edge to orange-white at the centre
            // (artistic rendering of a hot, compressed core; brightens in bursts).
            // Heating follows the collapse itself (compression), then the embedded source.
            float squeeze = max(uCloud.z, uCloud.y * 0.7) * (1.0 + 0.12 * sin(TIME * 2.3));
            float hot = squeeze * exp(-r / 2.8);
            emission += mix(vec3(0.9, 0.18, 0.05), vec3(1.0, 0.62, 0.30), exp(-r / 1.4)) * 2.4 * hot * min(d, 2.0);

            // Interstellar dust grains: visible albedo ~0.5, slightly bluer when
            // scattering (reddening comes from EXTINCTION_TINT on the light paths).
            vec3 albedo = vec3(0.56, 0.50, 0.45);
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
