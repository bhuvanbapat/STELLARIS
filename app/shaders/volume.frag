#version 450
// Ray-marched volumetric molecular cloud (rendered at reduced resolution).
//
// Density: procedural filament + dense core + clumps, eroded by 3D noise.
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

const float CLOUD_R = 46.0;
const vec3  KEY_DIR = normalize(vec3(-0.55, 0.62, 0.40));  // toward the external light
const vec3  EXTINCTION_TINT = vec3(0.80, 1.0, 1.22);       // dust extinguishes blue more

vec4 noiseAt(vec3 p) { return texture(uNoise, p); }

// Large-scale cloud structure (no small-scale detail).
float cloudShape(vec3 p) {
    // Curved filament through the origin.
    vec3 axis = normalize(vec3(1.0, 0.22, -0.38));
    float s = dot(p, axis);
    vec3 perp = p - axis * s;
    perp += vec3(0.0, 4.0 * sin(s * 0.06), 3.0 * cos(s * 0.045 + 1.0));
    float width = 9.0 + 3.0 * sin(s * 0.09 + 2.0);
    float filament = exp(-dot(perp, perp) / (2.0 * width * width)) * exp(-pow(s / 44.0, 2.0));

    float core = exp(-dot(p, p) / (2.0 * 7.5 * 7.5));
    vec3 c1 = p - vec3(-20.0, 7.0, 10.0);
    vec3 c2 = p - vec3(22.0, -6.0, -8.0);
    vec3 c3 = p - vec3(6.0, 12.0, 18.0);
    float clumps = 0.7 * exp(-dot(c1, c1) / 110.0) + 0.6 * exp(-dot(c2, c2) / 90.0) +
                   0.35 * exp(-dot(c3, c3) / 70.0);
    return max(filament * 1.1 + core * 0.75, clumps);
}

// Undisturbed cloud density at position p (before collapse mapping).
float baseDensity(vec3 p, float drift) {
    vec3 warpP = p * 0.010 + vec3(0.0, 0.0, drift);
    vec3 warp = (noiseAt(warpP).gba - 0.5) * 14.0;
    vec3 q = p + warp;
    float shape = cloudShape(q);
    if (shape < 0.02) return 0.0;
    vec4 n1 = noiseAt(q * 0.016 + vec3(drift * 0.5, 0.0, 0.0));
    vec4 n2 = noiseAt(q * 0.052 - vec3(0.0, drift, 0.0));
    vec4 n3 = noiseAt(q * 0.13 + vec3(0.0, 0.0, drift));
    // Billowy cells plus ridged filaments; the fine octave erodes the edges.
    float detail = n1.r * 0.58 + n2.g * 0.26 + n2.a * 0.16 + (n3.a - 0.5) * 0.16;
    float d = saturate(remap(detail, 1.0 - shape * 0.85, 1.0, 0.0, 1.0));
    return d * d * 2.2;
}

// Collapse mapping and outflow carving. Returns density at current position p.
float density(vec3 p) {
    float collapse = uCloud.y;
    float drift = TIME * 0.0012;

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

    float d = baseDensity(q, drift) * min(sqrt(jac), 4.0);

    // Inner envelope is accreted / cleared as the disc and star form.
    float clear = uCloud.w;
    d *= mix(1.0, smoothstep(2.0, 14.0, r), clear * 0.85);
    // Bipolar outflow cavities along the rotation axis.
    float cosAxis = abs(p.y) / max(r, 1e-3);
    float coneCos = cos(radians(mix(8.0, 34.0, clear)));
    d *= mix(1.0, 0.06, clear * smoothstep(coneCos - 0.06, coneCos + 0.04, cosAxis) * smoothstep(1.0, 4.0, r));
    // Feedback from the star blows the envelope outward and thins it.
    float disp = uCloud2.x;
    d *= 1.0 - disp * 0.75 * exp(-r / 18.0);

    return d * uCloud.x;
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

float phaseHG(float cosTheta, float g) {
    float g2 = g * g;
    return (1.0 - g2) / (4.0 * PI * pow(1.0 + g2 - 2.0 * g * cosTheta, 1.5));
}

// Optical depth along a short segment (4 samples).
float opticalDepth(vec3 from, vec3 dir, float len) {
    float tau = 0.0;
    const float f[4] = float[](0.08, 0.22, 0.45, 0.80);
    for (int i = 0; i < 4; ++i) tau += density(from + dir * (f[i] * len));
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

    vec2 hit = sphereHit(ro, rd, CLOUD_R);
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
    float stepLen = 2.0 * CLOUD_R / float(uSteps);
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

    for (int i = 0; i < uSteps && t < tFar; ++i) {
        vec3 p = ro + rd * t;
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
            vec3 toC = -p / max(r, 1e-3);
            float tauC = opticalDepth(p, toC, r * 0.92) * SIGMA;
            float fall = centralPower / (1.0 + r * r * 0.035);
            float phaseC = phaseHG(dot(rd, toC), 0.45) * 4.0 * PI * 0.6 + 0.4;
            vec3 lightC = glowColor * fall * exp(-tauC * EXTINCTION_TINT * 1.6) * phaseC;

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
