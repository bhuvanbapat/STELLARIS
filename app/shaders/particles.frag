#version 450
// Soft Gaussian point sprite, additive. Moving particles are drawn as a streak:
// a capsule along their on-screen motion, brightest at the leading end.
in vec3 vColor;
in float vAlpha;
in vec4 vStreak;  // direction xy (sprite space), half length, half width (both in d units)
out vec4 outColor;
void main() {
    vec2 d = gl_PointCoord * 2.0 - 1.0;
    float w;
    if (vStreak.z > 0.0) {
        float h = vStreak.z, rw = vStreak.w;
        float u = clamp(dot(d, vStreak.xy), -h, h);
        vec2 off = d - vStreak.xy * u;
        float r2 = dot(off, off) / (rw * rw);
        if (r2 > 1.0) discard;
        float head = 0.2 + 0.8 * (u + h) / (2.0 * h);       // fades toward the tail
        // Spread the same energy over the longer footprint.
        w = exp(-r2 * 4.0) * head * vAlpha / (1.0 + 0.6 * h / rw);
    } else {
        float r2 = dot(d, d);
        if (r2 > 1.0) discard;
        w = exp(-r2 * 4.0) * vAlpha;
    }
    outColor = vec4(vColor * w, 0.0);
}
