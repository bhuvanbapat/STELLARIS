#version 450
// Additive pulsar-beam glow, brightest near the neutron star.
#include "common.glsl"
in vec3 vWorld;
in float vAlong;   // 0 at the star, 1 at the far end
out vec4 outColor;
uniform vec3 uAxis;
uniform float uIntensity;
void main() {
    vec3 v = normalize(uCamPos.xyz - vWorld);
    // A cone surface seen side-on shows two bright silhouette lines. Weighting by
    // how obliquely we look along the axis makes it read as a soft volume instead.
    float through = sqrt(saturate(1.0 - abs(dot(v, uAxis))));
    float fade = (1.0 - vAlong) * (1.0 - vAlong) * smoothstep(0.0, 0.03, vAlong);
    outColor = vec4(vec3(0.55, 0.75, 1.0) * fade * through * uIntensity, 0.0);
}
