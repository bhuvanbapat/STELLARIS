#version 450
// 13-tap downsample (Jimenez 2014, "Next Generation Post Processing in Call of
// Duty: Advanced Warfare"). The first pass applies a Karis average per 2x2
// block to suppress fireflies from single very bright pixels.
in vec2 vUV;
out vec4 outColor;
layout(binding = 0) uniform sampler2D uSrc;
uniform vec2 uSrcTexel;
uniform int uFirst;

float luma(vec3 c) { return dot(c, vec3(0.2126, 0.7152, 0.0722)); }
float karisWeight(vec3 c) { return 1.0 / (1.0 + luma(c)); }

void main() {
    vec2 t = uSrcTexel;
    vec3 a = texture(uSrc, vUV + t * vec2(-2, 2)).rgb;
    vec3 b = texture(uSrc, vUV + t * vec2(0, 2)).rgb;
    vec3 c = texture(uSrc, vUV + t * vec2(2, 2)).rgb;
    vec3 d = texture(uSrc, vUV + t * vec2(-2, 0)).rgb;
    vec3 e = texture(uSrc, vUV).rgb;
    vec3 f = texture(uSrc, vUV + t * vec2(2, 0)).rgb;
    vec3 g = texture(uSrc, vUV + t * vec2(-2, -2)).rgb;
    vec3 h = texture(uSrc, vUV + t * vec2(0, -2)).rgb;
    vec3 i = texture(uSrc, vUV + t * vec2(2, -2)).rgb;
    vec3 j = texture(uSrc, vUV + t * vec2(-1, 1)).rgb;
    vec3 k = texture(uSrc, vUV + t * vec2(1, 1)).rgb;
    vec3 l = texture(uSrc, vUV + t * vec2(-1, -1)).rgb;
    vec3 m = texture(uSrc, vUV + t * vec2(1, -1)).rgb;

    vec3 res;
    if (uFirst == 1) {
        vec3 g0 = (a + b + d + e) * 0.25, g1 = (b + c + e + f) * 0.25;
        vec3 g2 = (d + e + g + h) * 0.25, g3 = (e + f + h + i) * 0.25;
        vec3 g4 = (j + k + l + m) * 0.25;
        float w0 = karisWeight(g0) * 0.125, w1 = karisWeight(g1) * 0.125;
        float w2 = karisWeight(g2) * 0.125, w3 = karisWeight(g3) * 0.125;
        float w4 = karisWeight(g4) * 0.5;
        res = (g0 * w0 + g1 * w1 + g2 * w2 + g3 * w3 + g4 * w4) / (w0 + w1 + w2 + w3 + w4);
    } else {
        res = e * 0.125 + (a + c + g + i) * 0.03125 + (b + d + f + h) * 0.0625 + (j + k + l + m) * 0.125;
    }
    outColor = vec4(max(res, 0.0), 1.0);
}
