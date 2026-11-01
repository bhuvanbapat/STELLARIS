#version 450
// Accretion disc shading (premultiplied alpha).
//  - Surface density falls off ~1/r with a soft outer taper.
//  - Temperature follows the steady thin-disc profile T ~ r^-3/4; color is the
//    blackbody color of that temperature (brightness is compressed artistically).
//  - Structure is advected with Keplerian angular velocity omega ~ r^-3/2, so
//    inner regions visibly orbit faster and patterns shear into spirals.
//  - Ring-like gaps are an artistic nod to structures observed in young discs.
#include "common.glsl"

in vec3 vWorld;
in float vRadius;
in float vAngle;
flat in int vLayer;
out vec4 outColor;
layout(binding = 0) uniform sampler3D uNoise;
uniform int uLayers;

const float KEPLER_K = 5.0;  // angular velocity at r = 1 (rad per timeline second)

void main() {
    float presence = uDisc.x;
    float inner = uDisc.z, outer = max(uDisc.y, inner + 0.1);
    float r = vRadius;

    // Keplerian advection. The disc clock starts when the disc forms.
    float discTime = max(TIME - 46.0, 0.0);
    float omega = KEPLER_K * pow(r, -1.5);
    float phi = vAngle - omega * discTime;
    vec3 sp = vec3(cos(phi) * r, float(vLayer) * 0.7, sin(phi) * r);
    vec4 n1 = texture(uNoise, sp * 0.055 + vec3(0.13, 0.0, 0.31));
    vec4 n2 = texture(uNoise, sp * 0.16 + vec3(0.0, 0.5, 0.0));
    float pattern = 0.55 + 0.75 * n1.r + 0.35 * (n2.a - 0.5);

    // Surface density with inner and outer edges, and ring gaps.
    float sigma = pow(r / inner, -0.9);
    sigma *= smoothstep(inner, inner * 1.25, r);
    sigma *= exp(-pow(r / outer, 3.0) * 1.2);
    float g1 = 0.42 * outer, g2 = 0.68 * outer;
    sigma *= 1.0 - 0.55 * exp(-pow((r - g1) / (0.035 * outer), 2.0));
    sigma *= 1.0 - 0.45 * exp(-pow((r - g2) / (0.03 * outer), 2.0));
    sigma *= pattern;

    // T ~ r^-3/4, anchored at the inner edge.
    float temp = 3800.0 * pow(r / inner, -0.75);
    vec3 thermal = blackbody(temp) * (0.25 + pow(temp / 3800.0, 2.2) * 3.0);

    // Dust on the disc surface scattering light from the central star.
    vec3 starLight = uCentralColor.rgb * uCentral.z * uCentral.x * 0.9 / (1.0 + r * r * 0.12);
    vec3 scattered = starLight * vec3(0.85, 0.80, 0.78) * 1.8;

    float layerWeight = 1.0 / float(uLayers);
    vec3 col = (thermal * 0.7 + scattered * 0.6) * sigma * layerWeight;
    float alpha = saturate(sigma * 0.45) * layerWeight * 0.6;

    // Fade where the disc meets the edges of the grid (no hard rim).
    col *= presence;
    alpha *= presence;
    outColor = vec4(col, saturate(alpha));
}
