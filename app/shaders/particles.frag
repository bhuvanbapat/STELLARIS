#version 450
// Soft Gaussian point sprite, additive.
in vec3 vColor;
in float vAlpha;
out vec4 outColor;
void main() {
    vec2 d = gl_PointCoord * 2.0 - 1.0;
    float r2 = dot(d, d);
    if (r2 > 1.0) discard;
    float w = exp(-r2 * 4.0) * vAlpha;
    outColor = vec4(vColor * w, 0.0);
}
