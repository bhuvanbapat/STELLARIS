#version 450
// Restrained explanatory overlay color, distinct from emitted plasma; fades with
// distance so the lines stay subordinate to the star.
in float vAcross;
in float vRadius;
out vec4 outColor;
uniform float uIntensity;
uniform float uFadeRadius;   // radius (in stellar radii) where the lines fade out
void main() {
    float across = exp(-vAcross * vAcross * 2.0);
    float fade = 1.0 - smoothstep(uFadeRadius * 0.5, uFadeRadius, vRadius);
    outColor = vec4(vec3(0.55, 0.78, 1.0) * across * fade * uIntensity, 0.0);
}
