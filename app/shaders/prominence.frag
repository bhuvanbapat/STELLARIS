#version 450
// Emissive prominence ribbon: soft across its width, slightly brighter at the top.
in float vAcross;
in float vAlong;
in float vOffDisc;
out vec4 outColor;
uniform float uIntensity;
void main() {
    float across = exp(-vAcross * vAcross * 3.0);
    float lift = 0.6 + 0.4 * sin(3.14159265 * vAlong);
    float visibility = mix(0.12, 1.0, vOffDisc);
    outColor = vec4(vec3(1.0, 0.34, 0.40) * across * lift * visibility * uIntensity, 0.0);
}
