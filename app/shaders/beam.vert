#version 450
// Pulsar beam: an open cone oriented along a (rotating) magnetic axis.
#include "common.glsl"
layout(location = 0) in vec3 aPos;   // apex at origin, axis +Y, height 1, end radius 1
uniform mat4 uModel;
out vec3 vWorld;
out float vAlong;
void main() {
    vAlong = aPos.y;
    vec4 w = uModel * vec4(aPos, 1.0);
    vWorld = w.xyz;
    gl_Position = uViewProj * w;
}
