#pragma once
// Application shell: window and GL context, main loop, input, command line.

#include "render/Renderer.h"
#include "sim/CameraPath.h"
#include "sim/SceneState.h"
#include "sim/Timeline.h"
#include "gfx/ImageTexture.h"
#include "sim/ObservationCatalog.h"
#include "ui/Ui.h"

#include <filesystem>
#include <map>
#include <memory>
#include <string>
#include <vector>

struct GLFWwindow;

namespace stellaris {

struct AppOptions {
    int    width = 1600, height = 900;
    bool   fullscreen = false;
    double startTime = 0.0;
    bool   autoplay = false;
    ModelConfig config;
    bool   hideUi = false;
    // Capture mode: render each listed timeline time, save PNGs, then exit.
    std::filesystem::path captureDir;
    std::vector<double>   captureTimes;
    // Self-test mode: exercise playback/seek/reset and verify GL health, then exit.
    bool selfTest = false;
    // Benchmark mode: render sample points of every stage without vsync, report timings, exit.
    bool benchmark = false;
    QualityPreset quality = QualityPreset::Balanced;
    bool fieldLines = false;
    bool scientific = false;
    bool observation = false;
};

// Parses argv. Returns false (and prints usage) on invalid input.
bool parseOptions(int argc, char** argv, AppOptions& out);

class App {
public:
    explicit App(AppOptions options);
    ~App();
    App(const App&)            = delete;
    App& operator=(const App&) = delete;

    int run();  // process exit code

private:
    bool initWindow();
    void frame(double realDt);
    void renderScene();
    bool saveScreenshot(const std::filesystem::path& path);
    int  runCapture();
    int  runSelfTest();
    int  runBenchmark();
    void toggleFullscreen();
    void resetAll();
    void rebuildCloudIfNeeded(bool force);

    // GLFW callbacks (static trampolines -> instance).
    static void onKey(GLFWwindow* w, int key, int scancode, int action, int mods);
    static void onMouseButton(GLFWwindow* w, int button, int action, int mods);
    static void onCursorPos(GLFWwindow* w, double x, double y);
    static void onScroll(GLFWwindow* w, double dx, double dy);

    AppOptions  options_;
    GLFWwindow* window_ = nullptr;
    std::unique_ptr<Renderer> renderer_;  // destroyed before the GL context
    Ui          ui_;
    bool        uiReady_ = false;
    Playback    playback_;
    ModelConfig config_;
    SceneState  scene_;
    CloudField  cloud_;  // baked procedural cloud for config_.cloudSeed
    std::vector<Observation> observations_;              // attributed images (sidecar catalog)
    std::map<std::string, ImageTexture> observationTextures_;  // loaded on first use; freed before the GL context
    UserCameraOffset cameraOffset_;
    float       lensShift_ = 0.0f;

    bool   dragging_ = false;
    double lastX_ = 0.0, lastY_ = 0.0;
    double lastClickTime_ = -1.0;
    bool   screenshotRequested_ = false;
    float  fps_ = 0.0f;
    int    windowedPos_[2] = {100, 100};
    int    windowedSize_[2] = {1600, 900};
};

} // namespace stellaris
