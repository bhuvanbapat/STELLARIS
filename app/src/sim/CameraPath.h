#pragma once
// Authored cinematic camera, evaluated from timeline position (deterministic).
// The user's manual orbit/zoom is a separate offset layered on top, so it can
// never corrupt the authored path and is removed by "recenter" or reset.

#include "sim/StellarModel.h"

#include <glm/vec3.hpp>

namespace stellaris {

struct CameraPose {
    glm::vec3 position{0.0f, 0.0f, 100.0f};
    glm::vec3 target{0.0f};
    float     fovYDegrees = 50.0f;
};

// Orbit-style description of the camera relative to its target.
struct OrbitParams {
    float distance;      // scene units
    float azimuthDeg;    // around +Y
    float elevationDeg;  // above the XZ plane
    float fovYDegrees;
};

struct UserCameraOffset {
    float yawDeg   = 0.0f;
    float pitchDeg = 0.0f;
    float zoom     = 1.0f;  // multiplies distance; <1 moves closer

    bool isIdentity() const { return yawDeg == 0.0f && pitchDeg == 0.0f && zoom == 1.0f; }
};

// starVisualRadius scales the main-sequence framing so large stars stay in frame.
OrbitParams authoredOrbit(double time, Pathway pathway, float starVisualRadius);
CameraPose  poseFromOrbit(const OrbitParams& orbit, const UserCameraOffset& offset);

} // namespace stellaris
