// Cloud density shared by the volume renderer (volume.frag) and the scientific
// cross-section (section.frag), so the slice shows exactly the field that is drawn.
// Normalized, dimensionless procedural density (SCIENTIFIC_MODEL §20.1).
// Requires common.glsl. Binds: noise volume at unit 0, cloud structure at unit 2.
layout(binding = 0) uniform sampler3D uNoise;

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

