#include "sim/MagneticField.h"

#include <glm/geometric.hpp>
#include <glm/trigonometric.hpp>

#include <cmath>

namespace stellaris {

glm::vec3 dipoleField(const glm::vec3& r, const glm::vec3& m) {
    const float len = glm::length(r);
    if (len < 1e-6f) return glm::vec3(0.0f);  // singular at the origin
    const glm::vec3 rh = r / len;
    return (3.0f * glm::dot(m, rh) * rh - m) / (len * len * len);
}

namespace {
// Unit tangent of the field line; zero where the field vanishes.
glm::vec3 direction(const glm::vec3& p, const glm::vec3& m, float sign) {
    const glm::vec3 b = dipoleField(p, m);
    const float n = glm::length(b);
    return n > 1e-9f ? sign * b / n : glm::vec3(0.0f);
}
} // namespace

std::vector<glm::vec3> traceFieldLine(const glm::vec3& surfaceStart, const glm::vec3& moment,
                                      const FieldLineSettings& s) {
    std::vector<glm::vec3> line;
    glm::vec3 p = glm::normalize(surfaceStart) * s.stellarRadius;
    // Follow whichever orientation of B points away from the star at the footpoint.
    const float sign = glm::dot(dipoleField(p, moment), p) >= 0.0f ? 1.0f : -1.0f;
    line.push_back(p);
    const float h = s.step;
    for (int i = 0; i < s.maxSteps; ++i) {
        const glm::vec3 k1 = direction(p, moment, sign);
        if (glm::dot(k1, k1) == 0.0f) break;  // vanishing field: stop
        const glm::vec3 k2 = direction(p + 0.5f * h * k1, moment, sign);
        const glm::vec3 k3 = direction(p + 0.5f * h * k2, moment, sign);
        const glm::vec3 k4 = direction(p + h * k3, moment, sign);
        const glm::vec3 prev = p;
        p += (h / 6.0f) * (k1 + 2.0f * k2 + 2.0f * k3 + k4);
        const float r = glm::length(p);
        if (r < s.stellarRadius) {
            // Closed line: find where the last step crosses the surface (bisection
            // along the step chord). A radial snap would move the end point off the
            // field line by up to one step.
            float lo = 0.0f, hi = 1.0f;
            for (int b = 0; b < 30; ++b) {
                const float mid = 0.5f * (lo + hi);
                (glm::length(prev + (p - prev) * mid) > s.stellarRadius ? lo : hi) = mid;
            }
            line.push_back(prev + (p - prev) * hi);
            break;
        }
        line.push_back(p);
        if (r > s.maxRadius) break;  // open line (cut at the overlay boundary)
    }
    return line;
}

std::vector<std::vector<glm::vec3>> traceDipoleLines(const glm::vec3& moment, const std::vector<float>& latitudesDeg,
                                                     int longitudes, const FieldLineSettings& settings) {
    // Basis aligned with the dipole axis.
    const glm::vec3 axis = glm::normalize(moment);
    const glm::vec3 ref = std::abs(axis.y) < 0.9f ? glm::vec3(0, 1, 0) : glm::vec3(1, 0, 0);
    const glm::vec3 e1 = glm::normalize(glm::cross(ref, axis));
    const glm::vec3 e2 = glm::cross(axis, e1);
    std::vector<std::vector<glm::vec3>> lines;
    for (float latDeg : latitudesDeg) {
        const float lat = glm::radians(latDeg);
        for (int k = 0; k < longitudes; ++k) {
            const float lon = 6.2831853f * (static_cast<float>(k) + 0.5f * (latDeg > 60.0f ? 1.0f : 0.0f)) /
                              static_cast<float>(longitudes);
            const glm::vec3 dir = std::cos(lat) * (std::cos(lon) * e1 + std::sin(lon) * e2) + std::sin(lat) * axis;
            lines.push_back(traceFieldLine(dir, moment, settings));
        }
    }
    return lines;
}

} // namespace stellaris
