#version 450
// Prominence loops: a few arches anchored to the stellar surface near the active
// regions, drawn as camera-facing ribbons (triangle strip, one instance per loop).
// Shapes are prescribed curves that slowly "breathe"; illustrative, not a
// magnetic-field or plasma calculation.
#include "common.glsl"
#include "stellar_activity.glsl"

uniform float uRadius;
uniform int uSegments;

out float vAcross;  // -1..1 across the ribbon
out float vAlong;   // 0..1 along the arch
out float vOffDisc; // 0 when seen in front of the bright disc, 1 beyond the limb

void main() {
    int loop = gl_InstanceID;
    int seg = gl_VertexID / 2;
    float side = (gl_VertexID % 2 == 0) ? -1.0 : 1.0;
    float s = float(seg) / float(uSegments);

    // Footpoints straddle an active region (spots 0, 1, 3).
    const int spotOf[3] = int[](0, 1, 3);
    vec3 c = spotCenter(spotOf[loop]);
    vec3 east = normalize(cross(vec3(0.0, 1.0, 0.0), c));
    vec3 tilt = normalize(east + vec3(0.0, 0.35 * float(loop - 1), 0.0));
    float span = 0.24 + 0.05 * float(loop);
    vec3 footA = normalize(c - tilt * span);
    vec3 footB = normalize(c + tilt * span);

    float height = (0.30 + 0.08 * float(loop)) * (1.0 + 0.08 * sin(TIME * 0.35 + float(loop) * 2.1));
    vec3 dir = normalize(mix(footA, footB, s));
    float r = uRadius * (1.0 + height * sin(PI * s));
    vec3 center = worldFromStellar(dir, TIME) * r;

    // Ribbon offset perpendicular to the arch tangent and the view direction.
    float s2 = min(s + 0.01, 1.0), s1 = max(s - 0.01, 0.0);
    vec3 p2 = worldFromStellar(normalize(mix(footA, footB, s2)), TIME) * uRadius * (1.0 + height * sin(PI * s2));
    vec3 p1 = worldFromStellar(normalize(mix(footA, footB, s1)), TIME) * uRadius * (1.0 + height * sin(PI * s1));
    vec3 tangent = normalize(p2 - p1);
    vec3 toCam = normalize(uCamPos.xyz - center);
    vec3 offsetDir = normalize(cross(tangent, toCam));
    float width = uRadius * 0.042 * (0.7 + 0.6 * sin(PI * s));

    // Seen against the photosphere a prominence is outshone (it appears as a dark
    // filament on the real Sun); additive emission cannot darken, so suppress it there.
    vec3 view = normalize(center - uCamPos.xyz);
    vec3 toStar = -center;
    float behind = dot(view, toStar);
    float miss = length(cross(view, toStar)) / uRadius;
    vOffDisc = behind > 0.0 ? smoothstep(0.9, 1.05, miss) : 1.0;

    vAcross = side;
    vAlong = s;
    gl_Position = uViewProj * vec4(center + offsetDir * width * side, 1.0);
}
