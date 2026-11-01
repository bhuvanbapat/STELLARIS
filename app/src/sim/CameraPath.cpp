#include "sim/CameraPath.h"

#include <algorithm>
#include <cmath>
#include <vector>

namespace stellaris {

namespace {

struct Key {
    double t;
    float  distance, azimuthDeg, elevationDeg, fov;
};

// Uniform Catmull-Rom on scalar channels; gives C1-continuous motion through keys.
float catmullRom(float p0, float p1, float p2, float p3, float u) {
    const float u2 = u * u, u3 = u2 * u;
    return 0.5f * ((2.0f * p1) + (-p0 + p2) * u + (2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3) * u2 +
                   (-p0 + 3.0f * p1 - 3.0f * p2 + p3) * u3);
}

std::vector<Key> buildKeys(Pathway pathway, float starR) {
    // Stage 5 framing distance grows with the (symbolic) radius of the star.
    const float ms = 9.0f + 4.0f * starR;
    std::vector<Key> k = {
        // Stage 1: wide establishing shot of the cloud, slow drift.
        {0.0, 96.0f, -24.0f, 11.0f, 50.0f},
        {18.0, 82.0f, -10.0f, 13.0f, 50.0f},
        // Stage 2: push in toward the collapsing core.
        {30.0, 62.0f, 2.0f, 15.0f, 48.0f},
        {40.0, 46.0f, 12.0f, 18.0f, 46.0f},
        // Stage 3: close on the protostar, rising to reveal the forming disc plane.
        {50.0, 34.0f, 22.0f, 21.0f, 45.0f},
        {58.0, 30.0f, 30.0f, 22.0f, 45.0f},
        // Stage 4: oblique view showing disc ellipse and vertical jets together.
        {70.0, 30.0f, 44.0f, 17.0f, 45.0f},
        {82.0, 24.0f, 62.0f, 13.0f, 45.0f},
        // Stage 5: portrait of the main-sequence star.
        {89.0, ms * 1.1f, 74.0f, 9.0f, 42.0f},
        {100.0, ms, 88.0f, 8.0f, 42.0f},
    };
    // Stage 6 (timeline 100-148) depends on the evolutionary pathway.
    auto at = [](double u) { return 100.0 + 48.0 * u; };
    switch (pathway) {
    case Pathway::WhiteDwarf:
        k.push_back({at(0.28), 26.0f, 108.0f, 12.0f, 44.0f});
        k.push_back({at(0.56), 34.0f, 126.0f, 16.0f, 44.0f});
        k.push_back({at(1.00), 44.0f, 150.0f, 20.0f, 44.0f});
        break;
    case Pathway::NeutronStar:
        k.push_back({at(0.25), 34.0f, 104.0f, 11.0f, 44.0f});
        k.push_back({at(0.31), 37.0f, 108.0f, 12.0f, 44.0f});
        k.push_back({at(0.50), 62.0f, 120.0f, 16.0f, 46.0f});
        k.push_back({at(0.75), 50.0f, 134.0f, 17.0f, 46.0f});
        k.push_back({at(1.00), 30.0f, 148.0f, 14.0f, 44.0f});
        break;
    case Pathway::BlackHole:
        k.push_back({at(0.25), 38.0f, 104.0f, 11.0f, 44.0f});
        k.push_back({at(0.31), 41.0f, 108.0f, 12.0f, 44.0f});
        k.push_back({at(0.50), 62.0f, 120.0f, 14.0f, 46.0f});
        k.push_back({at(0.75), 34.0f, 134.0f, 10.0f, 44.0f});
        k.push_back({at(1.00), 17.0f, 146.0f, 6.0f, 42.0f});
        break;
    }
    return k;
}

} // namespace

OrbitParams authoredOrbit(double time, Pathway pathway, float starVisualRadius) {
    const std::vector<Key> keys = buildKeys(pathway, starVisualRadius);
    const double t = std::clamp(time, keys.front().t, keys.back().t);

    std::size_t i = 0;
    while (i + 2 < keys.size() && t > keys[i + 1].t) ++i;
    const Key& k1 = keys[i];
    const Key& k2 = keys[i + 1];
    const Key& k0 = keys[i == 0 ? 0 : i - 1];
    const Key& k3 = keys[std::min(i + 2, keys.size() - 1)];
    const float u = static_cast<float>((t - k1.t) / (k2.t - k1.t));

    // Distance is interpolated in log space so zooms feel uniform.
    OrbitParams o{};
    o.distance = std::exp(catmullRom(std::log(k0.distance), std::log(k1.distance),
                                     std::log(k2.distance), std::log(k3.distance), u));
    o.azimuthDeg   = catmullRom(k0.azimuthDeg, k1.azimuthDeg, k2.azimuthDeg, k3.azimuthDeg, u);
    o.elevationDeg = catmullRom(k0.elevationDeg, k1.elevationDeg, k2.elevationDeg, k3.elevationDeg, u);
    o.fovYDegrees  = catmullRom(k0.fov, k1.fov, k2.fov, k3.fov, u);
    return o;
}

CameraPose poseFromOrbit(const OrbitParams& orbit, const UserCameraOffset& offset) {
    constexpr float kDeg = 3.14159265f / 180.0f;
    const float az = (orbit.azimuthDeg + offset.yawDeg) * kDeg;
    const float el = std::clamp(orbit.elevationDeg + offset.pitchDeg, -85.0f, 85.0f) * kDeg;
    const float d  = orbit.distance * std::clamp(offset.zoom, 0.25f, 4.0f);

    CameraPose p;
    p.target      = glm::vec3(0.0f);
    p.position    = glm::vec3(d * std::cos(el) * std::sin(az), d * std::sin(el), d * std::cos(el) * std::cos(az));
    p.fovYDegrees = orbit.fovYDegrees;
    return p;
}

} // namespace stellaris
