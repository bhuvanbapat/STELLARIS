#version 450
// Background: procedural star field and a faint galactic dust band, evaluated
// per pixel from the view direction (so it has correct parallax-free depth).
#include "common.glsl"

in vec2 vUV;
out vec4 outColor;
layout(binding = 0) uniform sampler3D uNoise;
// Real sky: ESO/S. Brunier all-sky Milky Way panorama (eso0932a, CC BY 4.0), a
// plate-carree map in galactic coordinates with the galactic centre in the middle
// and longitude increasing to the left. Sampled as sRGB, so values are linear.
layout(binding = 1) uniform sampler2D uSkyMap;
uniform int uHasSkyMap;

// Scene-to-galactic frame: the galactic plane is placed along the same tilted great
// circle as the procedural band, so the stellar nursery sits against the Milky Way
// (molecular clouds lie in the galactic disc).
const vec3 GAL_NORTH = normalize(vec3(0.25, 1.0, -0.4));
const vec3 GAL_CENTRE_HINT = vec3(0.3, 0.0, -1.0);
const float SKY_GAIN = 0.16;     // photograph (display-referred) -> scene radiance

vec3 skyPhoto(vec3 dir, float pixelAngle) {
    vec3 gc = normalize(GAL_CENTRE_HINT - GAL_NORTH * dot(GAL_CENTRE_HINT, GAL_NORTH));
    vec3 east = cross(GAL_NORTH, gc);
    float l = atan(dot(dir, east), dot(dir, gc));          // galactic longitude
    float b = asin(clamp(dot(dir, GAL_NORTH), -1.0, 1.0)); // galactic latitude
    vec2 uv = vec2(0.5 - l / (2.0 * PI), 0.5 - b / PI);
    // Explicit LOD from the angular pixel size: implicit derivatives jump at the
    // longitude seam and would draw a line of the smallest mip there.
    float texelsPerRad = float(textureSize(uSkyMap, 0).x) / (2.0 * PI);
    float lod = max(0.0, log2(pixelAngle * texelsPerRad));
    return textureLod(uSkyMap, uv, lod).rgb * SKY_GAIN;
}

vec3 viewDir(vec2 uv) {
    vec4 farP = uInvViewProj * vec4(uv * 2.0 - 1.0, 1.0, 1.0);
    return normalize(farP.xyz / farP.w - uCamPos.xyz);
}

// One layer of stars on a 3D grid of cells around the unit sphere.
vec3 starLayer(vec3 dir, float cellsPerUnit, float density, float pixelAngle, float brightness) {
    // Each cell holds at most one star, jittered away from the cell borders so its
    // ~1 px footprint is not clipped.
    vec3 cell = floor(dir * cellsPerUnit);
    vec3 h = hash33(cell);
    if (h.x > density) return vec3(0.0);
    vec3 starPos = normalize((cell + 0.5 + (hash33(cell + 19.0) - 0.5) * 0.5) / cellsPerUnit);
    float ang = acos(clamp(dot(dir, starPos), -1.0, 1.0));
    float r = ang / (pixelAngle * 0.8);
    float m = pow(h.y, 6.0);                             // steep brightness distribution
    float tempK = mix(3200.0, 14000.0, pow(h.z, 1.6));
    return blackbody(tempK) * exp(-r * r) * (0.05 + 3.0 * m) * brightness;
}

void main() {
    vec3 dir = viewDir(vUV);
    float pixelAngle = 2.0 / (uProj[1][1] * uViewport.y);  // radians per pixel (approx.)

    vec3 col = vec3(0.0);
    if (uHasSkyMap != 0) {
        // The photograph carries the real star field and Milky Way. A sparse layer of
        // bright procedural point stars keeps star cores crisp at screen resolution.
        col = skyPhoto(dir, pixelAngle);
        col += starLayer(dir, 34.0, 0.10, pixelAngle, 0.6);
        outColor = vec4(col, 1.0);
        return;
    }
    col += starLayer(dir, 90.0, 0.10, pixelAngle, 0.9);
    col += starLayer(dir, 200.0, 0.05, pixelAngle, 0.5);
    col += starLayer(dir, 34.0, 0.10, pixelAngle, 1.6);

    // Galactic band: a great circle tilted across the sky, built from fBm.
    vec3 bandNormal = normalize(vec3(0.25, 1.0, -0.4));
    float lat = dot(dir, bandNormal);
    float band = exp(-lat * lat * 10.0);
    vec4 n = texture(uNoise, dir * 0.55 + 0.5);
    vec4 n2 = texture(uNoise, dir * 1.7 + 0.3);
    float glow = band * smoothstep(0.35, 0.8, n.r * 0.7 + n2.r * 0.5);
    float dust = band * smoothstep(0.45, 0.75, n2.a);
    vec3 bandCol = mix(vec3(0.020, 0.024, 0.045), vec3(0.045, 0.030, 0.040), n.b);
    col += bandCol * glow * 1.6;
    col *= 1.0 - 0.75 * dust;

    // Very faint tonal floor so space is deep navy rather than pure black.
    col += vec3(0.0016, 0.0022, 0.0045) * (0.7 + 0.6 * n.b);

    outColor = vec4(col, 1.0);
}
