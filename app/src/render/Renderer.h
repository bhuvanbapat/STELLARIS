#pragma once
// Frame renderer. Pass order:
//   1. sky (star field)                         -> HDR target
//   2. opaque: central star, compact remnant    -> HDR target + depth
//   3. volumetric cloud (reduced resolution)    -> volume target, composited over HDR
//   4. accretion disc (premultiplied alpha), particles, beams, halos (additive)
//   5. black-hole lensing (screen space, when present)
//   6. bloom mip chain, then exposure + ACES tone map + sRGB to the window

#include "gfx/GL.h"
#include "gfx/Geometry.h"
#include "gfx/Shader.h"
#include "render/ParticleSystem.h"
#include "sim/CameraPath.h"
#include "sim/SceneState.h"

#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>

#include <array>
#include <filesystem>
#include <optional>
#include <vector>

namespace stellaris {

class Renderer {
public:
    Renderer() = default;
    ~Renderer();
    Renderer(const Renderer&)            = delete;
    Renderer& operator=(const Renderer&) = delete;

    bool init(const std::filesystem::path& shaderDir, int width, int height);
    void resize(int width, int height);
    bool reloadShaders();
    // Uploads the baked cloud structure and rebuilds the particles that follow it.
    void setCloud(const CloudField& cloud);

    // lensShiftX shifts the projection center horizontally in NDC units, used to
    // compose the subject to the right of the information panel.
    void render(const SceneState& scene, const CameraPose& camera, float lensShiftX);

    // Screen-space pixel position (origin top-left) of a world point, if in front
    // of the camera. Uses the matrices of the most recent render().
    std::optional<glm::vec2> projectToScreen(const glm::vec3& world) const;

    float gpuFrameMs() const { return gpuMs_; }
    int   particleCount() const { return particles_.count(); }

private:
    struct Target {
        gl::Framebuffer fbo;
        gl::Texture     color;
        int             width = 0, height = 0;
    };

    void createTargets();
    void createNoiseVolume();
    void createBlackbodyLut();
    void uploadFrameUniforms(const SceneState& s, const CameraPose& cam, float lensShiftX);
    void drawFullscreen();
    void drawGlow(const glm::vec3& center, float size, float innerFrac, float falloff,
                  const glm::vec3& color, float intensity, bool rays);
    void bloom(GLuint source);

    std::filesystem::path shaderDir_;
    int width_ = 0, height_ = 0;

    // Shaders
    Shader noiseGen_, sky_, volume_, composite_, star_, glow_, disc_, particlesShader_, beam_, lens_,
        bloomDown_, bloomUp_, final_;
    std::vector<Shader*> allShaders_;

    // Geometry
    Mesh sphere_, annulus_, cone_;
    gl::VertexArray emptyVao_;
    ParticleSystem particles_;

    // Resources
    gl::Texture noise3d_;
    gl::Texture cloudStructure_;  // RGBA16F: envelope, filament, core, globule
    gl::Texture blackbodyLut_;
    gl::Buffer  frameUbo_;
    gl::Framebuffer hdrFbo_;
    gl::Texture hdrColor_, hdrDepth_;
    Target volumeTarget_;
    Target lensTarget_;
    std::vector<Target> bloomMips_;

    // GPU timing (ring of queries so we never stall waiting for a result)
    std::array<GLuint, 4> timeQueries_{};
    int queryFrame_ = 0;
    float gpuMs_ = 0.0f;

    glm::mat4 view_{1.0f}, proj_{1.0f};
    glm::vec3 camPos_{0.0f};
};

} // namespace stellaris
