#version 450
// Simplified gravitational lensing by a point mass, applied in screen space.
// A pixel at angular offset theta from the black hole shows light from the
// source direction beta = theta - thetaE^2 / theta (thin-lens point-mass
// equation). Pixels inside the shadow radius are dark. Sizes are symbolic.
#include "common.glsl"
in vec2 vUV;
out vec4 outColor;
layout(binding = 0) uniform sampler2D uScene;
uniform vec2 uCenterUV;     // black hole position on screen
uniform float uShadowPx;    // shadow radius in pixels
uniform float uEinsteinPx;  // Einstein radius in pixels
uniform float uPresence;

void main() {
    vec2 c = uCenterUV * uViewport.xy;
    vec2 d = vUV * uViewport.xy - c;
    float b = length(d);

    float shadow = uShadowPx * uPresence;
    float thetaE2 = uEinsteinPx * uEinsteinPx * uPresence;
    vec2 srcUV = (c + d * (1.0 - thetaE2 / max(b * b, 1.0))) * uViewport.zw;
    srcUV = 1.0 - abs(1.0 - abs(srcUV));   // mirror off-screen lookups back inside
    vec3 col = texture(uScene, srcUV).rgb;

    // Event-horizon shadow with a thin, faint photon-ring highlight.
    float edge = smoothstep(shadow * 0.96, shadow * 1.04, b);
    float ring = exp(-pow((b - shadow * 1.06) / max(shadow * 0.05, 0.5), 2.0)) * uPresence;
    outColor = vec4(col * edge + vec3(1.0, 0.78, 0.55) * ring * 0.9, 1.0);
}
