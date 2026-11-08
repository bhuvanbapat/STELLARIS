#version 450
// Upsamples the reduced-resolution volume result over the HDR scene
// (blend: ONE, ONE_MINUS_SRC_ALPHA; the volume output is premultiplied).
//
// Joint bilateral upsampling: each of the four nearest low-resolution texels is
// weighted by its bilinear weight AND by how close the scene depth it was rendered
// with is to the depth of this full-resolution pixel. Across the star's silhouette
// the texels from the other side get a tiny weight, so the cloud does not bleed
// over the limb (and the star does not punch a soft hole into the cloud).
#include "common.glsl"

in vec2 vUV;
out vec4 outColor;
layout(binding = 0) uniform sampler2D uVolume;
layout(binding = 1) uniform sampler2D uSceneDepth;  // full resolution

float linearDepth(float d) {
    float zNdc = d * 2.0 - 1.0;
    return uProj[3][2] / (zNdc + uProj[2][2]);  // distance along the view axis
}

void main() {
    vec2 lowSize = vec2(textureSize(uVolume, 0));
    vec2 coord = vUV * lowSize - 0.5;
    vec2 base = floor(coord);
    vec2 f = coord - base;
    float zFull = linearDepth(texture(uSceneDepth, vUV).r);

    vec4 sum = vec4(0.0);
    float wsum = 0.0;
    for (int k = 0; k < 4; ++k) {
        vec2 o = vec2(k & 1, k >> 1);
        vec2 texel = clamp(base + o, vec2(0.0), lowSize - 1.0);
        vec2 uvLow = (texel + 0.5) / lowSize;
        float wb = (o.x > 0.5 ? f.x : 1.0 - f.x) * (o.y > 0.5 ? f.y : 1.0 - f.y);
        // The volume pass read the depth buffer at exactly this UV.
        float zLow = linearDepth(texture(uSceneDepth, uvLow).r);
        float wd = 1.0 / (1e-3 + abs(zLow - zFull) / max(zFull, 1e-3) * 40.0);
        float w = wb * wd + 1e-6;
        sum += texelFetch(uVolume, ivec2(texel), 0) * w;
        wsum += w;
    }
    outColor = sum / wsum;
}
