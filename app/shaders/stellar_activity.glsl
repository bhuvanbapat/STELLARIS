// Shared definitions for the star's surface activity (photosphere spots, flare,
// prominence anchors, corona streamers). Positions live in the star's rotating
// frame so every layer agrees, and they depend only on time: seeking is exact.
// All of this is an illustrative, parametric representation.

const float STAR_SPIN = 0.02;  // rad per timeline second (slowed for presentation)

// World-space unit vector -> rotating stellar frame.
vec3 stellarFrame(vec3 n, float time) {
    float r = time * STAR_SPIN;
    return vec3(cos(r) * n.x - sin(r) * n.z, n.y, sin(r) * n.x + cos(r) * n.z);
}
// Rotating stellar frame -> world space (inverse of stellarFrame).
vec3 worldFromStellar(vec3 s, float time) {
    float r = -time * STAR_SPIN;
    return vec3(cos(r) * s.x - sin(r) * s.z, s.y, sin(r) * s.x + cos(r) * s.z);
}

// Unit vector from latitude/longitude (radians).
vec3 latLon(float lat, float lon) { return vec3(cos(lat) * cos(lon), sin(lat), cos(lat) * sin(lon)); }

// Active regions: the five strongest regions of the real Sun in Carrington rotation
// 2284 (May 6 - June 2 2024; SDO/HMI synoptic magnetogram, tools/bake_sun_magnetogram.py).
// Region 0 is AR 13664 (~S20), source of the May 2024 storms, rotated in longitude to
// the site where STELLARIS stages its flare.
const int SPOT_COUNT = 5;
vec3 spotCenter(int i) {
    const float lat[5] = float[](-0.342, 0.431, -0.235, 0.465, -0.143);
    const float lon[5] = float[](2.002, 1.072, 5.524, 2.781, 4.536);
    return latLon(lat[i], lon[i]);
}

// Real radial photospheric field |Br| (gauss) of the same rotation, as a lat-lon map
// (R: log-scaled |Br|, G: polarity). uHasMagnetogram = 0 when the map is unavailable.
layout(binding = 4) uniform sampler2D uMagnetogram;
uniform int uHasMagnetogram;
float magneticField(vec3 sp, float lod) {
    vec2 uv = vec2(fract(atan(sp.z, sp.x) / 6.2831853), 0.5 - asin(clamp(sp.y, -1.0, 1.0)) / 3.14159265);
    float r = textureLod(uMagnetogram, uv, lod).r;
    return pow(10.0, r * 3.47727) - 1.0;   // inverse of log10(1 + B) / log10(3001)
}
float spotRadius(int i) {
    const float r[5] = float[](0.085, 0.06, 0.045, 0.07, 0.04);
    return r[i];
}

// 0 (no spot) .. 1 (umbra); penumbra ~0.5. Edges perturbed by the noise volume.
float spotDarkening(vec3 sp, sampler3D noiseTex) {
    float wobble = texture(noiseTex, sp * 1.9 + vec3(3.1)).r - 0.5;
    float result = 0.0;
    for (int i = 0; i < SPOT_COUNT; ++i) {
        float ang = acos(clamp(dot(sp, spotCenter(i)), -1.0, 1.0));
        float r = spotRadius(i) * (1.0 + wobble * 0.5);
        float penumbra = 1.0 - smoothstep(r * 0.9, r * 1.6, ang);
        float umbra = 1.0 - smoothstep(r * 0.35, r * 0.6, ang);
        result = max(result, penumbra * 0.5 + umbra * 0.5);
    }
    return result;
}

// Flare site: just beside the largest spot (an active region).
vec3 flareCenter() { return normalize(spotCenter(0) + vec3(0.0, 0.06, 0.08)); }
float flareKernel(vec3 sp) {
    float ang = acos(clamp(dot(sp, flareCenter()), -1.0, 1.0));
    return exp(-pow(ang / 0.05, 2.0));
}
