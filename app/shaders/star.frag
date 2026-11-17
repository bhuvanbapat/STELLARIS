#version 450
// Stellar photosphere, after the observed Sun (SDO/HMI continuum, DKIST/SST
// granulation images) and, for giants, interferometric images and 3D convection
// models of red supergiants (e.g. Betelgeuse; Chiavassa et al. 2011):
//   - wavelength-dependent limb darkening: stronger in blue than in red, so the limb
//     is darker and redder (quadratic law with per-channel coefficients, of the order
//     measured for the Sun by Neckel & Labs 1994);
//   - granulation as cellular convection: bright, hot upflow cells separated by narrow,
//     dark, cooler intergranular lanes (Worley cell noise, evolving in time); intensity
//     contrast ~15% for the Sun, larger for giants, whose few cells span much of the disc;
//   - sunspots with a dark umbra and a filamentary penumbra; faculae (bright patches)
//     around active regions, visible mainly near the limb, as observed;
//   - a flare footpoint; a protostar seen through its dusty envelope.
// Compact remnants (white dwarf, neutron star) use the same shader with no convection.
// Parametric visual representation (SCIENTIFIC_MODEL §20.4): no MHD is simulated.
#include "common.glsl"
#include "stellar_activity.glsl"

in vec3 vNormal;
in vec3 vWorld;
out vec4 outColor;
layout(binding = 0) uniform sampler3D uNoise;

uniform vec3  uColor;
uniform float uIntensity;
uniform float uTurbulence;    // 0 = no convection (remnant), ~0.1-0.35 main sequence, 1 = giant
uniform float uVeil;          // protostar still wrapped in its envelope
uniform float uLimbU;         // overall limb-darkening strength (0.6 = solar)
uniform float uSeed;
uniform float uActivity;      // 0..1: spots/faculae/flare (convective, main-sequence star)
uniform float uFlare;         // 0..1 flare pulse
uniform float uPixelsPerRadius;  // projected size, used to fade sub-pixel detail

// 3D Worley noise: distances to the nearest and second-nearest feature point.
// Feature points drift slowly (granules evolve, new ones appear as old ones fade).
vec2 worley(vec3 p, float t) {
    vec3 c = floor(p);
    vec3 f = p - c;
    float d1 = 8.0, d2 = 8.0;
    for (int z = -1; z <= 1; ++z)
        for (int y = -1; y <= 1; ++y)
            for (int x = -1; x <= 1; ++x) {
                vec3 o = vec3(x, y, z);
                vec3 h = hash33(c + o + uSeed * 17.0);
                vec3 pt = o + 0.5 + 0.38 * sin(t * (0.6 + h * 0.8) + h * 6.2831853) ;
                vec3 d = pt - f;
                float dd = dot(d, d);
                if (dd < d1) { d2 = d1; d1 = dd; }
                else if (dd < d2) { d2 = dd; }
            }
    return sqrt(vec2(d1, d2));
}

// Granulation intensity factor (mean ~1): bright cell centres, dark lanes.
float granulation(vec3 sp, float cellsPerRadius, float contrast, float t) {
    vec2 w = worley(sp * cellsPerRadius, t);
    float lane = smoothstep(0.0, 0.28, w.y - w.x);          // 0 in the lanes
    float dome = 1.0 - smoothstep(0.0, 0.85, w.x);           // hotter cell centres
    float g = 0.55 * lane + 0.45 * dome * lane;
    return 1.0 + contrast * (g - 0.42) * 2.4;
}

// Spots with penumbral filaments; returns darkening (0..1) and the facular mask.
vec2 spotsAndFaculae(vec3 sp) {
    if (uHasMagnetogram != 0) {
        // From the measured field: umbrae above ~1.2 kG, penumbrae ~0.5-1.2 kG with
        // radial-looking filaments, faculae/plage ~0.06-0.4 kG (typical solar values).
        float B = magneticField(sp, 0.0);
        float fil = 0.5 + 0.5 * sin(texture(uNoise, sp * 9.0).g * 40.0);
        float umbra = smoothstep(1100.0, 1700.0, B);
        float penumbra = smoothstep(420.0, 900.0, B) * (0.38 + 0.24 * fil);
        float plage = smoothstep(60.0, 260.0, magneticField(sp, 1.5)) * (1.0 - smoothstep(380.0, 600.0, B));
        return vec2(max(penumbra, umbra * 0.92), plage);
    }
    float wobble = texture(uNoise, sp * 1.9 + vec3(3.1)).r - 0.5;
    float dark = 0.0, fac = 0.0;
    for (int i = 0; i < SPOT_COUNT; ++i) {
        vec3 c = spotCenter(i);
        float ang = acos(clamp(dot(sp, c), -1.0, 1.0));
        float r = spotRadius(i) * (1.0 + wobble * 0.5);
        // Azimuth around the spot centre for radial penumbral filaments.
        vec3 tA = normalize(cross(c, vec3(0.0, 1.0, 0.0)));
        vec3 tB = cross(c, tA);
        float az = atan(dot(sp, tB), dot(sp, tA));
        float fil = 0.5 + 0.5 * sin(az * 46.0 + 3.0 * texture(uNoise, sp * 7.0).g);
        float penumbra = (1.0 - smoothstep(r * 0.9, r * 1.6, ang)) * (0.38 + 0.24 * fil);
        float umbra = 1.0 - smoothstep(r * 0.35, r * 0.6, ang);
        dark = max(dark, max(penumbra, umbra * 0.92));
        fac = max(fac, (1.0 - smoothstep(r * 1.6, r * 4.2, ang)) * smoothstep(r * 1.2, r * 1.9, ang));
    }
    return vec2(dark, fac);
}

void main() {
    vec3 n = normalize(vNormal);
    vec3 v = normalize(uCamPos.xyz - vWorld);
    float mu = saturate(dot(n, v));
    float om = 1.0 - mu;

    // Quadratic limb darkening, per channel (blue darkens most). Giants: stronger.
    float k = uLimbU / 0.6 * (1.0 + 0.25 * uTurbulence);
    vec3 a = vec3(0.40, 0.50, 0.62) * k;
    vec3 b = vec3(0.22, 0.20, 0.17) * k;
    vec3 limb = max(vec3(1.0) - a * om - b * om * om, vec3(0.04));

    vec3 sp = stellarFrame(n, TIME);
    float t = TIME;

    // Convection: many small granules on main-sequence stars, a handful of giant
    // cells on giants. Detail that would be sub-pixel fades to its mean.
    // Hot main-sequence stars (turbulence ~0.08) have radiative envelopes: no
    // surface granulation. Sun-like stars (~0.35), protostars and giants convect.
    float conv = smoothstep(0.12, 0.25, uTurbulence);
    // Real scale: the Sun has ~10^6 granules (~1 Mm across, ~700 per solar radius),
    // which are sub-pixel at this distance and blend into a fine mottling; a red
    // supergiant has only a few giant cells (~3 per radius) that dominate its disc.
    float cellsPerRadius = 700.0 * pow(3.2 / 700.0, smoothstep(0.35, 1.0, uTurbulence));
    float cellPixels = uPixelsPerRadius / cellsPerRadius;
    // A protostar is seen through its dusty envelope: no surface pattern shows.
    float detail = smoothstep(1.5, 5.0, cellPixels) * conv * (1.0 - smoothstep(0.05, 0.4, uVeil));
    float contrast = mix(0.15, 0.45, uTurbulence);
    float pattern = mix(1.0, granulation(sp, cellsPerRadius, contrast, t * 0.08), detail);
    // Mesoscale and, on giants, very large-scale brightness variations.
    vec4 m = texture(uNoise, sp * mix(0.9, 0.35, uTurbulence) + vec3(uSeed, t * 0.003, 0.0));
    pattern *= 1.0 + conv * (m.r - 0.5) * mix(0.10, 0.55, uTurbulence);
    // Granules seen at the limb lose contrast (corrugated surface, projection).
    pattern = mix(1.0, pattern, 0.35 + 0.65 * mu);
    // Lanes are cooler, hence redder.
    vec3 tint = mix(vec3(1.0, 0.70, 0.52), vec3(1.0), smoothstep(0.75, 1.05, pattern));

    // Active regions: spots (cooler: darker and redder) and faculae (bright near the limb).
    vec2 sf = spotsAndFaculae(sp) * uActivity;
    vec3 spotTint = mix(vec3(1.0), vec3(1.0, 0.70, 0.48), sf.x);
    float spotDim = 1.0 - 0.85 * sf.x;
    float faculae = 1.0 + sf.y * pow(om, 1.5) * 0.65;

    // Gravity darkening of fast-rotating hot stars (von Zeipel; observed by
    // interferometry on Altair and Vega): hotter, brighter, bluer poles.
    float radiative = (1.0 - conv) * step(0.02, uTurbulence);
    float pole = sp.y * sp.y - 0.33;
    vec3 gravity = vec3(1.0) + radiative * pole * vec3(0.16, 0.20, 0.26);

    vec3 col = uColor * tint * pattern * limb * spotTint * spotDim * faculae * gravity * uIntensity;

    // Flare footpoint: a localized, short-lived brightening beside the main spot.
    float flare = flareKernel(sp) * uFlare;
    col += vec3(1.0, 0.92, 0.85) * flare * uIntensity * 3.0;

    // A young protostar is seen through its dusty envelope: dimmer and redder.
    vec3 veiled = vec3(1.0, 0.35, 0.12) * uIntensity * 0.45 * (0.6 + 0.4 * pattern) * limb.r;
    col = mix(col, veiled, uVeil);

    outColor = vec4(col, 1.0);
}
