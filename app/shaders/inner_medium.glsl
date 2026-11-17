// Fine-scale gas structures close to the star, rendered as continuous media
// (emission + extinction) and ray-marched with short sub-steps inside the main
// volume loop (volume.frag). They replace the earlier particle streams and the
// layered disc geometry. Shapes follow observations and published simulations;
// they are prescribed density fields, not hydrodynamic solutions (SCIENCE_NOTES §9.8).
//
//   jets      HH 211 / HH 212 / HH 47 / HH 34: highly collimated beam (half-opening
//             ~2-3 deg), chains of knots that are internal working surfaces produced
//             by variations of the ejection velocity (each a small bow), a terminal bow
//             shock with a Mach disk at its apex, and a turbulent cocoon of entrained gas
//             (Raga et al. 1990; Hartigan et al. 2011; Reipurth & Bally 2001).
//   disc      HH 30 / IRAS 04302 (Hubble, Webb): a flared dusty disc whose opaque
//             midplane shows as a dark lane edge-on, lit surface layers above and below
//             (Chiang & Goldreich 1997); rings and gaps as in ALMA images of HL Tau.
//   streamers infalling arcs feeding the disc from the envelope (Pineda et al. 2020).
//   giant     extended molecular/dust layer around a red giant or supergiant
//             (e.g. VLT/SPHERE images of Betelgeuse and R Doradus).

const float JET_SPEED = 6.6;  // presentation units per timeline second

void jetMedium(vec3 p, inout vec3 emit, inout float sigma) {
    float on = uDisc.w;
    float len = uMisc.x;
    if (on <= 0.0 || len <= 0.1) return;
    float launch = uCentral.y * 1.3;
    float tip = len + launch;
    float ay = abs(p.y);
    if (ay > tip + 1.5) return;
    float side = p.y < 0.0 ? -1.0 : 1.0;
    // Slow precession of the jet axis: an S-shaped wiggle along the flow.
    float wig = sin(TIME * 0.35 - min(ay, tip) * 0.12) * 0.028 * min(ay, tip);
    vec2 q = vec2(p.x - wig, p.z);
    float r2 = dot(q, q);
    float Rc = 0.9 + 0.05 * len;                    // bow curvature radius
    float rBow = 1.3 + 0.07 * len;
    if (r2 > (rBow + 1.2) * (rBow + 1.2)) return;
    float r = sqrt(r2);

    float rb = 0.10 + 0.018 * ay;                   // beam radius (collimated)
    // Turbulence advected outward with the flow.
    vec4 nt = noiseAt(vec3(q.x * 1.4, ay * 0.8 - TIME * JET_SPEED * 0.12, q.y * 1.4) + side * 7.13);
    float turb = 0.55 + 0.9 * nt.r;
    float alongBeam = smoothstep(launch * 0.6, launch * 1.6, ay) * (1.0 - smoothstep(tip - 0.6, tip + 0.1, ay));

    // Continuous beam: faint; most of the emission comes from shocks.
    float beam = exp(-r2 / (rb * rb)) * alongBeam;

    // Internal working surfaces: one per ejection-velocity pulse. Ejection times are
    // irregular (hashed jitter) and pulses differ in strength; each knot is a small bow.
    float knots = 0.0;
    const float PERIOD = 0.85;  // ejection pulses: a long chain of knots, as in HH 211
    float n0 = floor(TIME / PERIOD);
    for (int k = 0; k < 8; ++k) {
        float n = n0 - float(k);
        float h = fract(sin(n * 12.9898 + side * 4.1) * 43758.5453);
        float age = TIME - (n * PERIOD + h * 0.7);
        float yk = launch + age * JET_SPEED;
        if (age < 0.0 || yk > tip - 0.3) continue;
        float dy = ay - yk + r2 / (2.0 * (0.30 + 0.035 * yk));
        float w = 0.05 + 0.004 * yk;
        knots += (0.45 + 0.9 * h) * exp(-dy * dy / w) * exp(-r2 / (rb * rb * 2.4));
    }

    // Terminal bow shock: paraboloid apex at the jet head, emission concentrated in a
    // thin shocked layer, brightest near the apex, fragmenting along the wings.
    float d = (tip - ay) - r2 / (2.0 * Rc);
    float shellW = 0.10 + 0.05 * r;
    float bow = exp(-d * d / (shellW * shellW)) * exp(-r / rBow) * step(-0.3, tip - ay + 0.6);
    float frag = 0.55 + 0.9 * texture(uNoise, vec3(q.x * 0.35, ay * 0.12, q.y * 0.35) + side * 3.1).b;
    // Mach disk: where the jet itself is decelerated, just behind the apex on axis.
    float mach = exp(-pow((ay - (tip - 0.22)) / 0.10, 2.0)) * exp(-r2 / (rb * rb * 1.6));

    // Cocoon: entrained, turbulent ambient gas sheathing the beam.
    float rc = rb * 3.2 + 0.08 * ay;
    float cocoon = exp(-r2 / (rc * rc)) * smoothstep(launch, tip * 0.5, ay) * (1.0 - smoothstep(tip - 0.4, tip + 0.3, ay));
    cocoon *= smoothstep(0.35, 0.75, nt.g);

    // Optical emission lines of shocked gas: H-alpha (rose) and [S II] (deep red);
    // the fastest shocks at the bow apex also show bluer, higher-excitation lines.
    vec3 hAlpha = vec3(1.0, 0.30, 0.38), sII = vec3(1.0, 0.16, 0.14), apex = vec3(1.0, 0.70, 0.82);
    float nearSrc = 1.0 - smoothstep(launch, launch + 3.5, ay);
    vec3 beamCol = mix(hAlpha, vec3(0.78, 0.84, 1.0), nearSrc * 0.6);
    vec3 e = beamCol * (0.25 * beam * turb + 5.0 * knots * turb)
           + mix(sII, apex, exp(-r2 * 1.5)) * (2.2 * bow * frag + 4.0 * mach)
           + vec3(0.85, 0.32, 0.20) * 0.04 * cocoon;
    emit += e * on;
    sigma += 0.06 * cocoon * on;
}

void discMedium(vec3 p, inout vec3 emit, inout float sigma) {
    float on = uDisc.x;
    if (on <= 0.0) return;
    float Rout = max(uDisc.y, 1.0);
    float Rin = max(uDisc.z, uCentral.y * 1.2);
    float R = length(p.xz);
    if (R > Rout * 1.45 || R < Rin * 0.7) return;
    // Flaring: the scale height grows faster than radius (H/R ~ R^0.25).
    float H = 0.035 + 0.075 * R * pow(R / Rout, 0.25);
    if (abs(p.y) > 5.0 * H) return;
    float Sigma = (Rin / R) * smoothstep(Rin * 0.8, Rin * 1.15, R) * (1.0 - smoothstep(Rout * 0.7, Rout * 1.35, R));
    // Rings and gaps at fixed fractions of the outer radius (after ALMA HL Tau).
    float gaps = 1.0 - 0.75 * exp(-pow((R - 0.42 * Rout) / (0.035 * Rout), 2.0))
                     - 0.60 * exp(-pow((R - 0.66 * Rout) / (0.030 * Rout), 2.0))
                     - 0.50 * exp(-pow((R - 0.86 * Rout) / (0.025 * Rout), 2.0));
    // Keplerian shear: texture rotates with omega ~ R^-3/2, so it winds into spirals.
    float phi = atan(p.z, p.x) - 5.0 * pow(max(R, 0.3), -1.5) * TIME;
    vec4 nd = noiseAt(vec3(cos(phi) * R * 0.45, p.y * 1.5, sin(phi) * R * 0.45) + vec3(3.7, 0.0, 1.9));
    float rho = Sigma * gaps * (0.7 + 0.6 * nd.r) / H * exp(-0.5 * p.y * p.y / (H * H)) * on;
    sigma += rho * 1.1;  // dust: the midplane is optically thick
    // Thermal emission of the inner disc (T ~ R^-3/4), dim beyond a few Rin.
    float T = 1500.0 * pow(R / Rin, -0.75);
    emit += blackbody(T) * rho * 0.9 * pow(Rin / R, 2.5);
    // Starlight scattered by the flared surface layers above and below the midplane.
    float surf = smoothstep(1.4 * H, 2.8 * H, abs(p.y));
    float star = uCentral.x * uCentral.z;
    emit += uCentralColor.rgb * star * rho * surf * 0.55 / (1.0 + R * R * 0.06);
}

void streamerMedium(vec3 p, inout vec3 emit, inout float sigma) {
    float on = uInner.x;
    if (on <= 0.0) return;
    float Rd = max(uDisc.y, 2.2);
    float R = length(p.xz);
    if (R > 15.0 || R < Rd * 0.6) return;
    // The arms descend from |y| = 0.32 (R - Rd); nothing exists outside that slab.
    if (abs(abs(p.y) - 0.32 * max(R - Rd, 0.0)) > 0.9 + 0.3 * R) return;
    float phi = atan(p.z, p.x);
    const float B = 0.32;  // log-spiral pitch
    float rho = 0.0, shock = 0.0;
    for (int j = 0; j < 2; ++j) {
        float phi0 = float(j) * PI + 0.7;
        float target = phi0 + log(R / Rd) / B;
        float dphi = mod(phi - target + PI, 2.0 * PI) - PI;
        float arc = dphi * R;
        float yArm = (j == 0 ? 1.0 : -1.0) * 0.32 * (R - Rd);  // falls in from above / below
        float w = 0.28 + 0.09 * R;
        float d2 = arc * arc + (p.y - yArm) * (p.y - yArm);
        if (d2 > 9.0 * w * w) continue;
        // Clumpy material flowing inward along the arm.
        vec4 nf = noiseAt(vec3(log(R) * 2.2 + TIME * 0.35, arc * 0.6, p.y * 0.6) + float(j) * 5.3);
        float arm = exp(-d2 / (w * w)) * (0.4 + 1.0 * nf.r);
        rho += arm;
        shock += arm * exp(-pow((R - Rd) / 0.6, 2.0));  // where the streamer lands on the disc
    }
    float fade = smoothstep(Rd * 0.7, Rd * 1.15, R) * (1.0 - smoothstep(11.0, 15.0, R));
    rho *= fade * on * 0.55;
    sigma += rho * 0.9;
    float star = uCentral.x * uCentral.z + uCloud.z;
    emit += (uCentralColor.rgb * star * 0.10 / (1.0 + R * R * 0.04) * vec3(0.9, 0.8, 0.75)) * rho;
    emit += vec3(1.0, 0.42, 0.16) * 0.12 * shock * fade * on;
}

void giantAtmosphere(vec3 p, inout vec3 emit, inout float sigma) {
    float on = uInner.y;
    float Rs = uCentral.y;
    if (on <= 0.0 || Rs <= 0.01) return;
    float r = length(p);
    if (r < Rs * 0.98 || r > Rs * 2.4) return;
    float h = (r - Rs) / Rs;
    vec3 dir = p / r;
    vec4 ng = noiseAt(dir * 1.6 + vec3(0.0, TIME * 0.006, 0.0));
    vec4 ng2 = noiseAt(dir * 4.0 + vec3(h * 0.6, 0.0, TIME * 0.004));
    // Molecular layer: steep falloff, large plumes lifted by the giant convection cells.
    float mol = exp(-h / (0.10 + 0.12 * ng.r)) * (0.6 + 0.8 * ng.g);
    // Dust forming farther out in clumps.
    float dust = exp(-pow((h - 0.85) / 0.35, 2.0)) * smoothstep(0.55, 0.78, ng2.r);
    float rho = (mol + 0.35 * dust) * on;
    sigma += rho * 1.6 / Rs;
    float star = uCentral.x * uCentral.z;
    emit += blackbody(2400.0) * mol * on * 0.5 * star / Rs;
    emit += uCentralColor.rgb * dust * on * star * 0.25 / Rs;
}

// Circumstellar rings, after SN 1987A (Hubble, Chandra, Webb; Sugerman et al. 2005):
// an equatorial ring and two fainter outer rings in an hourglass, shed by the progenitor
// long before the explosion. The explosion's UV flash ionizes them (they glow, then fade).
// When the blast wave reaches the equatorial ring, dense knots light up one after another
// as a "string of pearls" (hot spots, first seen ~1995-2001), then fade. Ring radius is
// chosen so the shock arrives ~5 s of timeline after the explosion. Illustrative timing.
const float RING_TILT = 0.75;   // radians: shows the ring as an ellipse, as SN 1987A is seen
void circumstellarRings(vec3 p, inout vec3 emit) {
    float on = uInner.w;
    if (on <= 0.0) return;
    float c = uEjecta.y;
    float strength = uEjecta2.x;
    float Rr = snShockRadius(5.0, strength);
    float cs = cos(RING_TILT), sn = sin(RING_TILT);
    vec3 q = vec3(p.x, cs * p.y - sn * p.z, sn * p.y + cs * p.z);
    if (dot(q, q) > (1.5 * Rr + 1.5) * (1.5 * Rr + 1.5)) return;
    float flash = exp(-c / 6.0);                         // flash-ionized glow, fading
    float rho = length(q.xz);
    float phi = atan(q.z, q.x);
    // Equatorial ring (Halpha/[N II] red when ionized).
    float d = length(vec2(rho - Rr, q.y));
    float ring = exp(-d * d / 0.03);
    // Hot spots: ~30 dense knots on the inner edge, lit as the shock reaches them.
    float jitter = texture(uNoise, vec3(phi * 0.6, 0.3, 0.7)).r;
    float cell = floor((phi + PI) / (2.0 * PI) * 30.0);
    float hp = fract(sin(cell * 91.7) * 43758.5);       // per-knot brightness / presence
    float pearls = pow(0.5 + 0.5 * cos(phi * 30.0 + jitter * 9.0), 14.0) * step(0.35, hp) * (0.3 + hp);
    float dIn = length(vec2(rho - Rr * 0.97, q.y));
    float knots = pearls * exp(-dIn * dIn / 0.05);
    float Rs = snShockRadius(c, strength);
    float hit = smoothstep(Rr * 0.92, Rr * 1.0, Rs + 0.4 * (jitter - 0.5));
    float since = max(c - 5.0, 0.0);
    float glowHot = hit * (0.35 + 0.65 * exp(-since / 14.0));
    // Outer rings: larger, displaced above and below, fainter, never shocked here.
    float dU = length(vec2(length(q.xz) - 1.3 * Rr, q.y - 0.85 * Rr));
    float dL = length(vec2(length(q.xz) - 1.3 * Rr, q.y + 0.85 * Rr));
    float outer = exp(-dU * dU / 0.02) + exp(-dL * dL / 0.02);
    vec3 red = vec3(1.0, 0.30, 0.30);
    emit += on * (red * ring * (0.02 + 0.25 * flash + 0.25 * glowHot)
                + vec3(1.0, 0.94, 0.88) * knots * glowHot * 6.0
                + red * outer * (0.01 + 0.07 * flash));
}

void innerMedium(vec3 p, inout vec3 emit, inout float sigma) {
    circumstellarRings(p, emit);
    jetMedium(p, emit, sigma);
    discMedium(p, emit, sigma);
    streamerMedium(p, emit, sigma);
    giantAtmosphere(p, emit, sigma);
}

// Radius of the sphere that contains every active inner structure.
float innerExtent() {
    float r = 0.0;
    if (uDisc.w > 0.0) r = max(r, uMisc.x + uCentral.y * 1.3 + 2.0);
    if (uDisc.x > 0.0) r = max(r, uDisc.y * 1.5);
    if (uInner.x > 0.0) r = max(r, 15.0);
    if (uInner.y > 0.0) r = max(r, uCentral.y * 2.45);
    if (uInner.w > 0.0) r = max(r, 1.5 * snShockRadius(5.0, uEjecta2.x) + 1.5);
    return r;
}
