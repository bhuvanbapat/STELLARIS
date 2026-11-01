#pragma once
// Dear ImGui interface: stage information, quantities, model parameters,
// transport controls, the six-stage timeline and 3D annotations.

#include "sim/SceneState.h"
#include "sim/Timeline.h"

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include <functional>
#include <optional>

struct GLFWwindow;
struct ImFont;

namespace stellaris {

struct UiFrameInfo {
    const SceneState* scene = nullptr;
    float fps = 0.0f;
    float gpuMs = 0.0f;
    bool  cameraOffsetActive = false;
    // Projects a world point to screen pixels (top-left origin), if visible.
    std::function<std::optional<glm::vec2>(const glm::vec3&)> project;
};

struct UiRequests {
    bool recenterCamera = false;
    bool resetAll       = false;
};

class Ui {
public:
    bool init(GLFWwindow* window, float contentScale);
    void shutdown();
    void beginFrame();
    UiRequests draw(Playback& playback, ModelConfig& config, const UiFrameInfo& info);
    void endFrame();

    bool wantsMouse() const;
    bool wantsKeyboard() const;
    bool visible() const { return visible_; }
    void toggleVisible() { visible_ = !visible_; }
    bool labelsEnabled() const { return labels_; }

    // Horizontal NDC lens shift that keeps the subject clear of the left panel.
    float compositionShift() const { return visible_ ? 0.14f : 0.0f; }

private:
    void drawInfoPanel(const SceneState& s);
    void drawParameterPanel(ModelConfig& config, const SceneState& s, UiRequests& req, bool cameraOffset);
    void drawTimeline(Playback& playback, const SceneState& s, UiRequests& req);
    void drawAnnotations(const UiFrameInfo& info);
    void drawDiagnostics(const UiFrameInfo& info);

    float  baseScale_ = 1.0f;  // monitor content scale (DPI)
    float  layout_ = 1.0f;     // shrink factor for small windows
    float  scale_ = 1.0f;      // baseScale_ * layout_, used for all UI metrics
    bool   visible_ = true;
    bool   labels_ = true;
    bool   initialized_ = false;
    ImFont* body_ = nullptr;
    ImFont* small_ = nullptr;
    ImFont* heading_ = nullptr;
    ImFont* title_ = nullptr;
};

} // namespace stellaris
