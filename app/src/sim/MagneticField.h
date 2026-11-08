#pragma once
// Analytic dipole magnetic field and field-line integration, used only for an
// explanatory overlay (SCIENTIFIC_MODEL §20.3).
//
//   B(r) = (mu0 / 4 pi r^3) [ 3 (m . r_hat) r_hat - m ]
//
// The mu0/4pi factor is dropped: field values are NORMALIZED (unit dipole moment,
// unit stellar radius). Field lines are mathematical integral curves of B/|B|:
// they are not particle trajectories, and nothing here couples the field to the
// plasma, the jets or the star's evolution. No MHD is involved.

#include <glm/vec3.hpp>

#include <vector>

namespace stellaris {

// Normalized dipole field of moment m at position r (|r| > 0). Returns 0 at r = 0.
glm::vec3 dipoleField(const glm::vec3& r, const glm::vec3& m);

struct FieldLineSettings {
    float stellarRadius = 1.0f;   // integration starts on and stops at this sphere
    float maxRadius     = 6.0f;   // lines leaving this sphere are cut (open lines)
    float step          = 0.02f;  // RK4 step length (in stellar radii)
    int   maxSteps      = 2000;   // hard cap on points per line
};

// Integrates one field line from a point on the stellar surface, following +B or
// -B (whichever leaves the surface), until it returns to the surface, exceeds
// maxRadius, reaches maxSteps, or meets a vanishing field. Deterministic.
std::vector<glm::vec3> traceFieldLine(const glm::vec3& surfaceStart, const glm::vec3& moment,
                                      const FieldLineSettings& settings);

// Fixed seed layout: `latitudes` x `longitudes` footpoints in the northern
// magnetic hemisphere (latitudes measured from the magnetic equator, degrees).
std::vector<std::vector<glm::vec3>> traceDipoleLines(const glm::vec3& moment,
                                                     const std::vector<float>& latitudesDeg,
                                                     int longitudes, const FieldLineSettings& settings);

} // namespace stellaris
