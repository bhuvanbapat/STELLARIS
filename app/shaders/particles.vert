#version 450
// Deterministic particle evaluation. Each particle stores only its initial
// position, its population, and four random numbers. Its position at any
// timeline time is computed in closed form here, so seeking is exact and no
// state is accumulated between frames.
//
// Populations:
//   0  ambient cloud gas: slow turbulent drift; later pushed out by feedback
//   1  dense-core gas: pressureless free fall (r = r0 cos^2 beta), conserving
//      angular momentum; lands on the disc at its centrifugal radius
//      r_c ~ j^2 (or accretes onto the star), then orbits with Keplerian
//      angular velocity while drifting inward (prescribed viscous drift)
//   2  jet gas: streams along the rotation axis with moving knots
//   3  final-stage ejecta: planetary-nebula shells or supernova ejecta
#include "common.glsl"

layout(location = 0) in vec4 aPos0;   // xyz initial position, w population
layout(location = 1) in vec4 aSeed;   // four uniform random numbers in [0,1)

out vec3 vColor;
out float vAlpha;

const float R_CORE = 13.0;      // radius of the collapsing core (scene units)
const float KEPLER_K = 5.0;     // must match disc.frag
const float T_VISC = 70.0;      // inward drift e-folding time (timeline s)

float kepler(float r) { return KEPLER_K * pow(max(r, 0.05), -1.5); }

vec3 turbulentDrift(vec3 p0, vec4 s, float t) {
    return vec3(sin(t * 0.07 * (0.5 + s.x) + s.y * 6.283),
                sin(t * 0.05 * (0.5 + s.z) + s.w * 6.283),
                cos(t * 0.06 * (0.5 + s.y) + s.x * 6.283)) * 0.6;
}

// Standard-normal-ish value from two uniforms (Box-Muller).
float gaussian(float u1, float u2) { return sqrt(-2.0 * log(max(u1, 1e-4))) * cos(6.2831853 * u2); }

// Solve (2/pi)(beta + sin(beta)cos(beta)) = f for beta in [0, pi/2] (bisection).
float freeFallBeta(float f) {
    float lo = 0.0, hi = 1.5707963;
    for (int i = 0; i < 18; ++i) {
        float mid = 0.5 * (lo + hi);
        float g = (2.0 / PI) * (mid + sin(mid) * cos(mid));
        if (g < f) lo = mid; else hi = mid;
    }
    return 0.5 * (lo + hi);
}

void ambient(vec3 p0, vec4 s, out vec3 p, out vec3 col, out float alpha, out float size) {
    float t = TIME;
    p = p0 + turbulentDrift(p0, s, t);
    // Stellar feedback pushes the remaining envelope outward and thins it.
    float disp = uCloud2.x;
    p *= 1.0 + disp * 0.45 * exp(-length(p0) / 30.0);
    float r = length(p);
    float key = 0.55 + 0.45 * dot(normalize(p0 + 1e-3), normalize(vec3(-0.45, 0.55, -0.70)));
    col = mix(vec3(0.20, 0.24, 0.48), vec3(0.42, 0.20, 0.42), s.z) * 0.03 * key;
    // Warm light from the central source on nearby gas.
    float src = uCentral.x * uCentral.z * 0.6 + uCloud.z * 0.4;
    col += mix(vec3(1.0, 0.45, 0.2), uCentralColor.rgb, uCentral.x) * src * 0.035 / (1.0 + r * r * 0.03);
    alpha = 0.35 * (1.0 - disp * 0.7) * uCloud.x;
    size = 0.15 + 0.15 * s.w;
}

void coreGas(vec3 p0, vec4 s, out vec3 p, out vec3 col, out float alpha, out float size) {
    float r0 = length(p0);
    float theta0 = acos(clamp(p0.y / max(r0, 1e-3), -1.0, 1.0));
    float phi0 = atan(p0.z, p0.x);
    float rcyl0 = r0 * sin(theta0);

    float starR = max(uCentral.y, 0.6);
    // Inside-out timing: free-fall time grows with starting radius (rho ~ r^-2).
    float tff = (3.0 + 3.2 * r0) * (0.85 + 0.3 * s.x);
    // Centrifugal radius r_c ~ j^2 with j ~ Omega * R_cyl^2 (uDiscScale ~ Omega^2).
    float rc = uCloud2.w * pow(rcyl0 / R_CORE, 4.0) * (0.7 + 0.6 * s.y);
    rc = min(rc, uCloud2.w * 1.1);
    bool accretes = rc < starR * 1.1;
    float rLand = max(rc, starR * 0.8);

    float tau = uCloud2.y;
    float f = tau / tff;
    float y0 = gaussian(s.z, s.w);
    vec3 baseCol = mix(vec3(0.22, 0.26, 0.50), vec3(0.45, 0.22, 0.45), s.z) * 0.022;

    if (f <= 0.0) {
        p = p0 + turbulentDrift(p0, s, TIME);
        col = baseCol;
        alpha = uCloud.x;
        size = 0.22;
        return;
    }

    float phi, r, theta, yOff = 0.0, driftWeight = 0.0;
    if (f < 1.0) {
        float beta = freeFallBeta(f);
        float x = cos(beta) * cos(beta);
        driftWeight = x;  // keeps the motion continuous with the pre-collapse drift
        r = rLand + (r0 - rLand) * x;
        theta = mix(theta0, 0.5 * PI, 1.0 - x);
        // phi(beta) = integral of j / r^2 dt, with dt/dbeta = tff (4/pi) cos^2(beta).
        float j = kepler(rLand) * rLand * rLand;
        float acc = 0.0;
        for (int k = 0; k < 8; ++k) {
            float b = beta * (float(k) + 0.5) / 8.0;
            float xb = cos(b) * cos(b);
            float rb = rLand + (r0 - rLand) * xb;
            acc += j / (rb * rb) * tff * (4.0 / PI) * xb;
        }
        phi = phi0 + acc * beta / 8.0;
        yOff = y0 * 0.045 * pow(rLand, 1.25) * (1.0 - x);
        // Heats up as it falls in.
        float heat = 1.0 - smoothstep(rLand, r0, r);  // 0 at the start, 1 on landing
        float tempK = mix(1200.0, 2600.0 * pow(max(rLand, 0.5) / 1.5, -0.6), heat);
        col = mix(baseCol, blackbody(tempK) * 0.10 * heat, heat);
        alpha = 1.0;
        if (accretes) alpha *= smoothstep(starR * 1.0, starR * 2.6, r);
        size = mix(0.22, 0.12, heat);
    } else {
        if (accretes) { p = vec3(0.0); col = vec3(0.0); alpha = 0.0; size = 0.0; return; }
        // Landed: Keplerian orbit with exponential inward drift.
        float since = tau - tff;
        float tv = T_VISC * (0.7 + 0.6 * s.w);
        r = rc * exp(-since / tv);
        // Angle accumulated during infall (beta -> pi/2), computed as above.
        float j = kepler(rc) * rc * rc;
        float acc = 0.0;
        for (int k = 0; k < 8; ++k) {
            float b = 1.5707963 * (float(k) + 0.5) / 8.0;
            float xb = cos(b) * cos(b);
            float rb = rc + (r0 - rc) * xb;
            acc += j / (rb * rb) * tff * (4.0 / PI) * xb;
        }
        float phiLand = phi0 + acc * 1.5707963 / 8.0;
        phi = phiLand + kepler(rc) * tv / 1.5 * (exp(1.5 * since / tv) - 1.0);
        theta = 0.5 * PI;
        yOff = y0 * 0.045 * pow(r, 1.25);
        float tempK = 3800.0 * pow(max(r, starR * 1.25) / (starR * 1.25), -0.75);
        col = blackbody(tempK) * (0.10 + 0.45 * pow(tempK / 3800.0, 2.0));
        alpha = uDisc.x * smoothstep(starR * 1.15, starR * 1.8, r);
        size = 0.10 + 0.06 * s.x;
    }
    p = vec3(r * sin(theta) * cos(phi), r * cos(theta) + yOff, r * sin(theta) * sin(phi));
    p += turbulentDrift(p0, s, TIME) * driftWeight;
}

void jetGas(vec4 s, out vec3 p, out vec3 col, out float alpha, out float size) {
    float presence = uDisc.w;
    float len = uMisc.x;
    if (presence <= 0.0 || len <= 0.1) { p = vec3(0.0); col = vec3(0.0); alpha = 0.0; size = 0.0; return; }
    float t = TIME;
    float rate = 0.11 + 0.04 * s.y;
    float phase = fract(s.x + t * rate);
    float side = s.z < 0.5 ? -1.0 : 1.0;
    float launch = uCentral.y * 1.3;
    float y = launch + phase * 17.0;
    if (y > len + launch) { p = vec3(0.0); col = vec3(0.0); alpha = 0.0; size = 0.0; return; }

    // Narrow, slightly widening stream with a slow precession of the axis.
    float bow = smoothstep(0.82, 1.0, y / (len + launch));
    float radius = (0.14 + 0.075 * y) * sqrt(fract(s.w * 7.13)) * (1.0 + 2.0 * bow);
    float a = s.w * 6.2831853;
    vec3 lateral = vec3(cos(a), 0.0, sin(a)) * radius;
    lateral.x += sin(t * 0.35 - y * 0.12) * 0.028 * y;
    lateral += (hash33(vec3(floor(y * 1.5 - t * 3.0), s.w * 97.0, side)) - 0.5) * 0.12 * y * 0.15;
    p = vec3(lateral.x, side * y, lateral.z);

    // Knots that move outward with the flow (episodic ejection).
    float knot = 0.35 + 0.65 * pow(0.5 + 0.5 * sin((y - 17.0 * rate * t) * 0.9 + s.z * 0.5), 3.0);
    col = mix(vec3(0.45, 0.70, 1.0), vec3(1.0, 0.36, 0.42), bow) * (0.04 + 0.14 * knot);
    alpha = presence * smoothstep(0.0, 0.06, phase) * (1.0 - smoothstep(0.9, 1.0, phase));
    size = 0.08 + 0.08 * knot + 0.2 * bow;
}

void ejecta(vec4 s, out vec3 p, out vec3 col, out float alpha, out float size) {
    float presence = uEjecta.x;
    if (presence <= 0.0) { p = vec3(0.0); col = vec3(0.0); alpha = 0.0; size = 0.0; return; }
    float clock = uEjecta.y;
    float z = s.y * 2.0 - 1.0;
    float a = s.z * 6.2831853;
    vec3 dir = vec3(sqrt(1.0 - z * z) * cos(a), z, sqrt(1.0 - z * z) * sin(a));

    if (uEjecta.z < 0.5) {
        // Planetary nebula: several shells shed in successive episodes, faster
        // along the axis (bipolar shape), glowing once ionized by the hot core.
        float shell = floor(s.x * 4.0);
        float age = clock - shell * 2.2 - s.w * 0.8;
        if (age <= 0.0) { p = vec3(0.0); col = vec3(0.0); alpha = 0.0; size = 0.0; return; }
        float v = (0.24 + 0.035 * shell) * (1.0 + 0.6 * pow(abs(dir.y), 2.0));
        vec3 clump = hash33(floor(dir * 6.0 + shell * 3.0));
        float r = 4.2 + v * age * (0.92 + 0.16 * clump.x);
        p = dir * r;
        float glow = uEjecta.w;
        float innerness = 1.0 - shell / 3.0;
        vec3 ionized = mix(vec3(1.0, 0.30, 0.24), vec3(0.20, 0.95, 0.80), innerness * 0.8 + 0.1 * clump.y);
        vec3 dusty = vec3(0.35, 0.16, 0.10) * 0.25;
        col = mix(dusty, ionized * (0.5 + 0.7 * clump.z), glow) * 0.08;
        alpha = presence * smoothstep(0.0, 1.5, age) * (0.55 + 0.45 * step(0.35, clump.y));
        size = 0.10 + 0.10 * clump.z;
    } else {
        // Supernova: clumpy, decelerating ejecta; hot and blue-white early,
        // then filaments in hydrogen-alpha red and oxygen teal (artistic).
        float strength = uEjecta2.x;
        if (s.w > mix(0.45, 1.0, saturate((strength - 0.35) / 0.65))) {
            p = vec3(0.0); col = vec3(0.0); alpha = 0.0; size = 0.0; return;
        }
        vec3 clumpDir = hash33(floor(dir * 5.0)) * 2.0 - 1.0;
        dir = normalize(dir + clumpDir * 0.35);
        float v = (0.55 + 0.45 * s.x) * mix(0.75, 1.0, strength);
        float age = clock;
        float r = 0.6 + 30.0 * v * (1.0 - exp(-age / 9.0));
        p = dir * r;
        float hot = exp(-age / 2.5);
        vec3 filament = mix(vec3(1.0, 0.32, 0.25), vec3(0.30, 0.85, 1.0), step(0.6, s.y));
        col = mix(filament * 0.12, vec3(0.9, 0.95, 1.0) * 1.5, hot) * strength;
        alpha = presence * (0.5 + 0.5 * smoothstep(0.3, 0.7, clumpDir.x * 0.5 + 0.5));
        size = 0.14 + 0.3 * hot + 0.08 * s.z;
    }
}

void main() {
    int pop = int(aPos0.w + 0.5);
    vec3 p, col;
    float alpha, size;
    if (pop == 0)      ambient(aPos0.xyz, aSeed, p, col, alpha, size);
    else if (pop == 1) coreGas(aPos0.xyz, aSeed, p, col, alpha, size);
    else if (pop == 2) jetGas(aSeed, p, col, alpha, size);
    else               ejecta(aSeed, p, col, alpha, size);

    gl_Position = uViewProj * vec4(p, 1.0);
    // World-space size -> pixels. Sub-pixel particles keep size 1 px and fade
    // their alpha with covered area, which avoids flicker.
    float px = size * uProj[1][1] * 0.5 * uViewport.y / max(gl_Position.w, 1e-3);
    float coverage = clamp(px * px, 0.0, 1.0);
    gl_PointSize = clamp(px, 1.0, 14.0);
    // Fade particles that pass very close to the camera so they never fill the screen.
    float camDist = length(p - uCamPos.xyz);
    float nearFade = smoothstep(4.0, 14.0, camDist);
    vColor = col;
    vAlpha = alpha * nearFade * (px < 1.0 ? coverage : 1.0) * (px > 14.0 ? 14.0 * 14.0 / (px * px) : 1.0);
    if (alpha <= 0.0) gl_Position = vec4(2.0, 2.0, 2.0, 1.0);  // clip
}
