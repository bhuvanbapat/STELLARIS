#version 450
// Upsamples the low-resolution volume result over the HDR scene
// (blend: ONE, ONE_MINUS_SRC_ALPHA; the volume output is premultiplied).
in vec2 vUV;
out vec4 outColor;
layout(binding = 0) uniform sampler2D uVolume;
void main() { outColor = texture(uVolume, vUV); }
