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
//   4  stellar wind: sparse radial outflow (main sequence; slow dusty wind of giants)
#include "common.glsl"

layout(location = 0) in vec4 aPos0;   // xyz initial position, w population
layout(location = 1) in vec4 aSeed;   // four uniform random numbers in [0,1)

uniform float uParticleFraction;  // render-quality subsampling (cost only)

out vec3 vColor;
out float vAlpha;
out vec4 vStreak;  // motion streak in sprite space: direction xy, half length, half width

// Motion streaks: every population is a pure function of time, so the position a
// moment earlier is evaluated exactly by shifting the clock by gDt. The sprite is
// stretched along the on-screen motion over STREAK_DT seconds of timeline.
float gDt = 0.0;
#undef TIME
#define TIME (uCamPos.w - gDt)
const float STREAK_DT = 0.12;

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

    float tau = uCloud2.y - gDt;  // infall clock runs at timeline rate
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
    // Fast flow: protostellar jets move at ~100-500 km/s, far faster than the
    // infalling gas, so the jet gas crosses its length in ~2-3 s of timeline.
    float rate = 0.34 + 0.10 * s.y;
    float phase = fract(s.x + t * rate);
    float side = s.z < 0.5 ? -1.0 : 1.0;
    float launch = uCentral.y * 1.3;
    float y = launch + phase * 17.0;
    if (y > len + launch) { p = vec3(0.0); col = vec3(0.0); alpha = 0.0; size = 0.0; return; }

    // Narrow, slightly widening stream with a slow precession of the axis.
    float bow = smoothstep(0.82, 1.0, y / (len + launch));
    // ~15% of the gas forms a hot, very narrow spine along the axis.
    bool spine = fract(s.w * 53.1) < 0.15;
    float radius = (0.14 + 0.075 * y) * sqrt(fract(s.w * 7.13)) * (1.0 + 2.0 * bow) * (spine ? 0.18 : 1.0);
    float a = s.w * 6.2831853;
    vec3 lateral = vec3(cos(a), 0.0, sin(a)) * radius;
    lateral.x += sin(t * 0.35 - y * 0.12) * 0.028 * y;
    lateral += (hash33(vec3(floor(y * 1.5 - t * 3.0), s.w * 97.0, side)) - 0.5) * 0.12 * y * 0.15;
    p = vec3(lateral.x, side * y, lateral.z);

    // Continuous, collimated beam. Internal shocks ("knots", as in Herbig-Haro jets)
    // are brightness enhancements that travel outward with the gas; the particles
    // themselves stay evenly spread so the beam never breaks into separate clumps.
    float kw = sin((y - 6.6 * t) * 0.8 + side * 1.3);
    float knot = pow(0.5 + 0.5 * kw, 4.0);
    // Colour after observed protostellar jets: hot, bluish-white gas near the
    // source, H-alpha/[S II] rose-red along the beam, deep red-orange bow shock.
    float along = smoothstep(0.0, 0.45, (y - launch) / max(len, 1e-3));
    vec3 beam = mix(vec3(0.70, 0.80, 1.0), vec3(1.0, 0.42, 0.52), along);
    // A quarter of the particles are large and faint: their overlap forms the diffuse
    // glow of the beam, so it reads as gas rather than a string of dots.
    bool halo = !spine && fract(s.w * 31.7) < 0.25;
    if (spine) beam = mix(vec3(0.85, 0.92, 1.0), beam, 0.5 * along);
    col = mix(beam, vec3(1.0, 0.36, 0.20), bow) * (0.045 + 0.08 * knot + 0.05 * bow) * (halo ? 0.35 : spine ? 1.8 : 1.0);
    alpha = presence * smoothstep(0.0, 0.06, phase) * (1.0 - smoothstep(0.9, 1.0, phase));
    size = (halo ? 0.38 : 0.13) + 0.06 * knot + 0.25 * bow;
}

void ejecta(vec4 s, out vec3 p, out vec3 col, out float alpha, out float size) {
    float presence = uEjecta.x;
    if (presence <= 0.0) { p = vec3(0.0); col = vec3(0.0); alpha = 0.0; size = 0.0; return; }
    float clock = uEjecta.y - gDt;  // ejecta clock runs at timeline rate
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
        // Supernova ejecta behind the shock front, stratified in three layers
        // (inner "oxygen-like" teal, middle "hydrogen-alpha-like" red, outer
        // blue-white), with ~30 % of the material in radial fingers along 48 fixed
        // directions. Instability-INSPIRED procedural morphology, not a solved
        // Rayleigh-Taylor flow; colors are artistic.
        float strength = uEjecta2.x;
        if (s.w > mix(0.45, 1.0, saturate((strength - 0.35) / 0.65))) {
            p = vec3(0.0); col = vec3(0.0); alpha = 0.0; size = 0.0; return;
        }
        vec3 r3 = hash33(vec3(s.x * 6151.0, s.y * 3967.0, s.z * 2843.0));
        float age = clock;
        float shock = snShockRadius(age, strength);
        bool finger = r3.x < 0.3;
        float frac;
        vec3 layerCol;
        if (finger) {
            vec3 fdir = normalize(hash33(vec3(floor(s.y * 48.0), 3.0, 7.0)) * 2.0 - 1.0);
            dir = normalize(fdir + (hash33(vec3(s.z * 977.0, s.w * 331.0, 5.0)) - 0.5) * 0.07);
            frac = mix(0.5, 0.97, r3.y);                 // spread along the finger
            layerCol = mix(vec3(1.0, 0.30, 0.24), vec3(0.75, 0.85, 1.0), smoothstep(0.75, 0.97, frac));
        } else {
            vec3 clumpDir = hash33(floor(dir * 5.0)) * 2.0 - 1.0;
            dir = normalize(dir + clumpDir * 0.3);
            float layer = r3.y;
            if (layer < 0.35)      { frac = mix(0.35, 0.60, r3.z); layerCol = vec3(0.30, 0.90, 0.65); }
            else if (layer < 0.75) { frac = mix(0.60, 0.85, r3.z); layerCol = vec3(1.0, 0.30, 0.24); }
            // Outer layer only in patches of directions: a full thin particle shell
            // limb-brightens into a uniform "bubble"; patches read as a broken rim.
            else if (hash33(floor(dir * 7.0) + 41.0).x < 0.35) { frac = mix(0.85, 0.97, r3.z); layerCol = vec3(0.60, 0.78, 1.0); }
            else                   { frac = mix(0.60, 0.85, r3.z); layerCol = vec3(1.0, 0.30, 0.24); }
        }
        p = dir * shock * snAsymmetry(dir) * frac;
        float hot = exp(-age / 2.0);
        float fade = 0.55 + 0.45 * exp(-age / 25.0);           // persistent but fading remnant
        col = mix(layerCol * 0.13 * fade, vec3(0.95, 0.95, 1.0) * 0.9, hot) * strength;
        alpha = presence;
        size = 0.11 + 0.18 * hot + 0.06 * s.z;
    }
}

void wind(vec4 s, out vec3 p, out vec3 col, out float alpha, out float size) {
    float strength = uWind.x;
    if (strength <= 0.0 || uCentral.y <= 0.0) { p = vec3(0.0); col = vec3(0.0); alpha = 0.0; size = 0.0; return; }
    // Each particle repeatedly leaves the surface along a fixed direction; the radius
    // grows linearly with phase (constant terminal speed, presentation scale).
    float R = uCentral.y;
    float span = 6.0 * R + 10.0;
    float phase = fract(s.x + TIME * uWind.y / span);
    float z = s.y * 2.0 - 1.0, az = s.z * 6.2831853;
    vec3 dir = vec3(sqrt(1.0 - z * z) * cos(az), z, sqrt(1.0 - z * z) * sin(az));
    vec3 clump = hash33(floor(dir * 9.0)) - 0.5;          // mild streams, not confetti
    dir = normalize(dir + clump * 0.18);
    p = dir * (R * 1.03 + phase * span);
    vec3 hot = mix(uCentralColor.rgb, vec3(0.75, 0.85, 1.0), 0.5);
    vec3 dusty = vec3(0.9, 0.45, 0.25);
    col = mix(hot, dusty, uWind.z) * 0.2;
    alpha = strength * smoothstep(0.0, 0.05, phase) * (1.0 - phase) * (1.0 - phase) * (0.4 + 0.6 * s.w);
    size = 0.07 + 0.08 * s.w;
}

void main() {
    int pop = int(aPos0.w + 0.5);
    vec3 p, col;
    float alpha, size;
    if (pop == 0)      ambient(aPos0.xyz, aSeed, p, col, alpha, size);
    else if (pop == 1) coreGas(aPos0.xyz, aSeed, p, col, alpha, size);
    else if (pop == 2) jetGas(aSeed, p, col, alpha, size);
    else if (pop == 3) ejecta(aSeed, p, col, alpha, size);
    else               wind(aSeed, p, col, alpha, size);

    // Earlier position for the motion streak (all moving populations).
    vec3 pPrev = p;
    if (alpha > 0.0 && (pop >= 1)) {
        vec3 c2; float a2, s2;
        gDt = STREAK_DT;
        if (pop == 1)      coreGas(aPos0.xyz, aSeed, pPrev, c2, a2, s2);
        else if (pop == 2) jetGas(aSeed, pPrev, c2, a2, s2);
        else if (pop == 3) ejecta(aSeed, pPrev, c2, a2, s2);
        else               wind(aSeed, pPrev, c2, a2, s2);
        gDt = 0.0;
        if (a2 <= 0.0 || distance(p, pPrev) > 6.0) pPrev = p;  // no streak across a respawn
    }

    gl_Position = uViewProj * vec4(p, 1.0);
    // World-space size -> pixels. Sub-pixel particles keep size 1 px and fade
    // their alpha with covered area, which avoids flicker.
    float px = size * uProj[1][1] * 0.5 * uViewport.y / max(gl_Position.w, 1e-3);
    float coverage = clamp(px * px, 0.0, 1.0);
    vec4 prevClip = uViewProj * vec4(pPrev, 1.0);
    vec2 motion = (gl_Position.xy / max(gl_Position.w, 1e-3) - prevClip.xy / max(prevClip.w, 1e-3)) * 0.5 * uViewport.xy;
    float streakLen = min(length(motion), 40.0);
    float pxc = clamp(px, 1.0, 14.0);
    vStreak = vec4(0.0);
    if (streakLen > 0.75 * pxc && prevClip.w > 0.0) {
        float sprite = streakLen + pxc;
        vec2 dir = motion / max(length(motion), 1e-4);
        vStreak = vec4(dir.x, -dir.y, streakLen / sprite, pxc / sprite);
        gl_PointSize = sprite;
    } else {
        gl_PointSize = pxc;
    }
    // Fade particles that pass very close to the camera so they never fill the screen.
    float camDist = length(p - uCamPos.xyz);
    float nearFade = smoothstep(4.0, 14.0, camDist);
    vColor = col;
    vAlpha = alpha * nearFade * (px < 1.0 ? coverage : 1.0) * (px > 14.0 ? 14.0 * 14.0 / (px * px) : 1.0);
    // Quality tiers draw a stable subset of particles and compensate brightness, so
    // the scene looks the same at lower cost (identity of the subset never changes).
    float keep = float(pcg3d(uvec3(uint(gl_VertexID), 7u, 11u)).x) * (1.0 / 4294967295.0);
    if (keep > uParticleFraction) alpha = 0.0;
    vAlpha /= max(uParticleFraction, 0.05);
    if (alpha <= 0.0) gl_Position = vec4(2.0, 2.0, 2.0, 1.0);  // clip
}
