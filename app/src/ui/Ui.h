#pragma once
// Dear ImGui interface: stage information, quantities, model parameters,
// transport controls, the six-stage timeline and 3D annotations.

#include "render/RenderQuality.h"
#include "sim/ObservationCatalog.h"
#include "sim/SceneState.h"
#include "sim/Timeline.h"

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include <functional>
#include <optional>

struct GLFWwindow;
struct ImFont;

namespace stellaris {

// One authentic observation ready for display (texture owned by the App).
struct ObservationItem {
    const Observation* observation = nullptr;
    unsigned texture = 0;
    int width = 0, height = 0;
};

struct UiFrameInfo {
    const SceneState* scene = nullptr;
    float fps = 0.0f;
    float gpuMs = 0.0f;
    bool  cameraOffsetActive = false;
    QualityPreset quality = QualityPreset::Balanced;
    std::vector<ObservationItem> observations;  // filled only in the Observation view
    // Projects a world point to screen pixels (top-left origin), if visible.
    std::function<std::optional<glm::vec2>(const glm::vec3&)> project;
};

struct UiRequests {
    bool recenterCamera = false;
    bool resetAll       = false;
    std::optional<QualityPreset> quality;  // render cost only
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
    bool fieldLinesEnabled() const { return fieldLines_; }
    bool tracersEnabled() const { return tracers_; }
    enum class ViewMode { Cinematic, Scientific, Observation };
    ViewMode viewMode() const { return mode_; }
    void setViewMode(ViewMode mode);
    bool scientificView() const { return mode_ == ViewMode::Scientific; }
    bool observationView() const { return mode_ == ViewMode::Observation; }
    void setScientificView(bool on) { setViewMode(on ? ViewMode::Scientific : ViewMode::Cinematic); }
    void setFieldLines(bool on) { fieldLines_ = on; }
    void setCompare(bool on) { compare_ = on; }
    void setSkyCredit(const char* credit) { skyCredit_ = credit; }
    // Sound preference: owned here so it survives stage changes; App persists it.
    bool audioMuted() const { return audioMuted_; }
    float audioVolume() const { return audioVolume_; }
    void setAudioPreference(bool muted, float volume) { audioMuted_ = muted; audioVolume_ = volume; }
    void toggleMute() { audioMuted_ = !audioMuted_; audioDirty_ = true; }
    bool takeAudioDirty() { const bool d = audioDirty_; audioDirty_ = false; return d; }

    // Horizontal NDC lens shift that keeps the subject clear of the left panel.
    float compositionShift() const { return visible_ ? (mode_ == ViewMode::Observation ? 0.42f : 0.14f) : 0.0f; }

private:
    void drawInfoPanel(const SceneState& s);
    void drawParameterPanel(ModelConfig& config, const SceneState& s, UiRequests& req, const UiFrameInfo& info);
    void drawTimeline(Playback& playback, const SceneState& s, UiRequests& req);
    void drawAnnotations(const UiFrameInfo& info);
    void drawDiagnostics(const UiFrameInfo& info);
    void drawSectionLegend(const SceneState& s);
    void drawObservationPanel(const UiFrameInfo& info);
    void drawComparisonPanel(ModelConfig& config);

    float  baseScale_ = 1.0f;  // monitor content scale (DPI)
    float  layout_ = 1.0f;     // shrink factor for small windows
    float  scale_ = 1.0f;      // baseScale_ * layout_, used for all UI metrics
    bool   visible_ = true;
    bool   labels_ = true;
    bool   fieldLines_ = false;
    bool   compare_ = false;   // mass-comparison table visible
    bool   tracers_ = false;   // optional gas tracer particles
    const char* skyCredit_ = nullptr;  // always-visible credit of the sky photograph
    ViewMode mode_ = ViewMode::Cinematic;
    int    observationIndex_ = 0;  // selected band when several observations match
    bool   labelClick_ = false;  // current timeline press started on a stage label
    bool   initialized_ = false;
    ImFont* body_ = nullptr;
    ImFont* small_ = nullptr;
    ImFont* heading_ = nullptr;
    ImFont* title_ = nullptr;
    ImFont* label_ = nullptr;
    bool   audioMuted_ = false;
    float  audioVolume_ = 0.7f;
    bool   audioDirty_ = false;  // preference changed since the last save
};

} // namespace stellaris
