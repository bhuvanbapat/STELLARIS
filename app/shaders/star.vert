#version 450
#include "common.glsl"
layout(location = 0) in vec3 aPos;
uniform vec3 uCenter;
uniform float uRadius;
out vec3 vNormal;
out vec3 vWorld;
void main() {
    vNormal = aPos;
    vWorld = uCenter + aPos * uRadius;
    gl_Position = uViewProj * vec4(vWorld, 1.0);
}
