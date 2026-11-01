#version 450
// 3x3 tent upsample, blended additively onto the next larger mip.
in vec2 vUV;
out vec4 outColor;
layout(binding = 0) uniform sampler2D uSrc;
uniform vec2 uSrcTexel;
void main() {
    vec2 t = uSrcTexel;
    vec3 s = texture(uSrc, vUV).rgb * 4.0;
    s += (texture(uSrc, vUV + vec2(-t.x, 0)).rgb + texture(uSrc, vUV + vec2(t.x, 0)).rgb +
          texture(uSrc, vUV + vec2(0, -t.y)).rgb + texture(uSrc, vUV + vec2(0, t.y)).rgb) * 2.0;
    s += texture(uSrc, vUV + vec2(-t.x, -t.y)).rgb + texture(uSrc, vUV + vec2(t.x, -t.y)).rgb +
         texture(uSrc, vUV + vec2(-t.x, t.y)).rgb + texture(uSrc, vUV + vec2(t.x, t.y)).rgb;
    outColor = vec4(s / 16.0, 1.0);
}
