#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#endif

#include "app/App.h"

#include "core/Log.h"
#include "core/PngWriter.h"
#include "gfx/GL.h"

#include <GLFW/glfw3.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <sstream>

#ifdef _WIN32
// Ask hybrid-graphics laptops to run this program on the discrete GPU.
extern "C" {
__declspec(dllexport) unsigned long NvOptimusEnablement = 1;
__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif

namespace stellaris {

namespace {

std::filesystem::path executableDir() {
#ifdef _WIN32
    wchar_t buf[MAX_PATH];
    const DWORD n = GetModuleFileNameW(nullptr, buf, MAX_PATH);
    if (n > 0 && n < MAX_PATH) return std::filesystem::path(buf).parent_path();
#endif
    return std::filesystem::current_path();
}

void printUsage() {
    std::printf(
        "STELLARIS - interactive visualization of stellar formation and evolution\n\n"
        "Usage: stellaris [options]\n"
        "  --width N --height N     window size (default 1600x900)\n"
        "  --fullscreen             start in borderless fullscreen\n"
        "  --time T                 start at timeline time T seconds (0-148)\n"
        "  --play                   start playing immediately\n"
        "  --mass M                 initial stellar mass in solar masses (1-40)\n"
        "  --rotation R             relative core rotation (0.6-1.4)\n"
        "  --seed N                 cloud morphology seed (default 1)\n"
        "  --quality Q              performance | balanced (default) | high | capture\n"
        "  --hide-ui                start with the interface hidden\n"
        "  --capture DIR T1,T2,...  render the listed times to DIR/*.png and exit\n"
        "  --selftest               run automated playback/seek checks and exit\n"
        "  --benchmark              measure frame times across all stages and exit\n");
}

void glfwErrorCallback(int code, const char* description) { log::error("GLFW error {}: {}", code, description); }

std::string timestampName() {
    const std::time_t now = std::time(nullptr);
    std::tm tm{};
#ifdef _WIN32
    localtime_s(&tm, &now);
#else
    localtime_r(&now, &tm);
#endif
    char buf[64];
    std::strftime(buf, sizeof(buf), "stellaris_%Y%m%d_%H%M%S.png", &tm);
    return buf;
}

} // namespace

bool parseOptions(int argc, char** argv, AppOptions& o) {
    auto number = [&](int& i, double& out) {
        if (i + 1 >= argc) return false;
        char* end = nullptr;
        out = std::strtod(argv[++i], &end);
        return end && *end == '\0';
    };
    for (int i = 1; i < argc; ++i) {
        const std::string a = argv[i];
        double v = 0.0;
        if (a == "--width" && number(i, v)) o.width = static_cast<int>(v);
        else if (a == "--height" && number(i, v)) o.height = static_cast<int>(v);
        else if (a == "--time" && number(i, v)) o.startTime = v;
        else if (a == "--mass" && number(i, v)) o.config.massSolar = std::clamp(v, kMinMassSolar, kMaxMassSolar);
        else if (a == "--seed" && number(i, v)) o.config.cloudSeed = static_cast<uint32_t>(std::max(v, 0.0));
        else if (a == "--rotation" && number(i, v))
            o.config.rotation = std::clamp(v, ModelConfig::kMinRotation, ModelConfig::kMaxRotation);
        else if (a == "--fullscreen") o.fullscreen = true;
        else if (a == "--play") o.autoplay = true;
        else if (a == "--hide-ui") o.hideUi = true;
        else if (a == "--selftest") o.selfTest = true;
        else if (a == "--benchmark") o.benchmark = true;
        else if (a == "--quality" && i + 1 < argc) {
            const std::string q = argv[++i];
            if (q == "performance") o.quality = QualityPreset::Performance;
            else if (q == "balanced") o.quality = QualityPreset::Balanced;
            else if (q == "high") o.quality = QualityPreset::High;
            else if (q == "capture") o.quality = QualityPreset::Capture;
            else { std::fprintf(stderr, "Unknown quality '%s'\n\n", q.c_str()); printUsage(); return false; }
        }
        else if (a == "--capture" && i + 2 < argc) {
            o.captureDir = argv[++i];
            std::stringstream ss(argv[++i]);
            std::string item;
            while (std::getline(ss, item, ',')) o.captureTimes.push_back(std::strtod(item.c_str(), nullptr));
        } else if (a == "--help" || a == "-h") {
            printUsage();
            return false;
        } else {
            std::fprintf(stderr, "Unknown or incomplete option: %s\n\n", a.c_str());
            printUsage();
            return false;
        }
    }
    o.width = std::clamp(o.width, 640, 7680);
    o.height = std::clamp(o.height, 360, 4320);
    return true;
}

App::App(AppOptions options) : options_(std::move(options)), config_(options_.config) {}

App::~App() {
    if (uiReady_) ui_.shutdown();
    if (window_) {
        renderer_.reset();  // GL resources must be released while the context is current
        glfwDestroyWindow(window_);
    }
    glfwTerminate();
}

bool App::initWindow() {
    glfwSetErrorCallback(glfwErrorCallback);
    if (!glfwInit()) {
        log::error("glfwInit failed.");
        return false;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#ifndef NDEBUG
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif
    glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_FALSE);
    glfwWindowHint(GLFW_DEPTH_BITS, 0);  // all 3D rendering happens in offscreen targets

    window_ = glfwCreateWindow(options_.width, options_.height, "STELLARIS", nullptr, nullptr);
    if (!window_) {
        log::error("Could not create an OpenGL 4.5 core window. Update the GPU driver.");
        return false;
    }
    glfwMakeContextCurrent(window_);
    if (!gladLoadGL(glfwGetProcAddress)) {
        log::error("Failed to load OpenGL functions.");
        return false;
    }
    log::info("OpenGL {} | {} | {}", reinterpret_cast<const char*>(glGetString(GL_VERSION)),
              reinterpret_cast<const char*>(glGetString(GL_RENDERER)),
              reinterpret_cast<const char*>(glGetString(GL_VENDOR)));
    gl::installDebugOutput();
    glfwSwapInterval(1);

    glfwSetWindowUserPointer(window_, this);
    // Our callbacks are installed before ImGui's so its backend chains to them.
    glfwSetKeyCallback(window_, onKey);
    glfwSetMouseButtonCallback(window_, onMouseButton);
    glfwSetCursorPosCallback(window_, onCursorPos);
    glfwSetScrollCallback(window_, onScroll);
    return true;
}

int App::run() {
    log::openLogFile((executableDir() / "stellaris.log").string().c_str());
    if (!initWindow()) return 1;

    int fbw = 0, fbh = 0;
    glfwGetFramebufferSize(window_, &fbw, &fbh);
    const std::filesystem::path shaderDir = executableDir() / "shaders";
    renderer_ = std::make_unique<Renderer>();
    if (!renderer_->init(shaderDir, fbw, fbh)) {
        log::error("Renderer initialization failed (shader directory: {}).", shaderDir.string());
        return 1;
    }
    renderer_->setQuality(options_.quality);
    rebuildCloudIfNeeded(true);
    float xscale = 1.0f, yscale = 1.0f;
    glfwGetWindowContentScale(window_, &xscale, &yscale);
    if (!ui_.init(window_, xscale)) return 1;
    uiReady_ = true;

    playback_.seek(options_.startTime);
    if (options_.hideUi) ui_.toggleVisible();
    if (options_.fullscreen) toggleFullscreen();
    lensShift_ = ui_.compositionShift();

    if (!options_.captureTimes.empty()) return runCapture();
    if (options_.selfTest) return runSelfTest();
    if (options_.benchmark) return runBenchmark();
    if (options_.autoplay) playback_.play();

    auto last = std::chrono::steady_clock::now();
    while (!glfwWindowShouldClose(window_)) {
        glfwPollEvents();
        const auto now = std::chrono::steady_clock::now();
        const double dt = std::chrono::duration<double>(now - last).count();
        last = now;
        frame(dt);
        glfwSwapBuffers(window_);
    }
    return 0;
}

void App::frame(double realDt) {
    playback_.update(realDt);
    if (realDt > 0.0) fps_ = fps_ * 0.95f + static_cast<float>(1.0 / realDt) * 0.05f;
    // Ease the composition shift when the UI is toggled (presentation only).
    const float target = ui_.compositionShift();
    lensShift_ += (target - lensShift_) * static_cast<float>(1.0 - std::exp(-realDt * 6.0));
    renderScene();

    if (screenshotRequested_) {
        screenshotRequested_ = false;
        const std::filesystem::path dir = std::filesystem::current_path() / "screenshots";
        std::error_code ec;
        std::filesystem::create_directories(dir, ec);
        const std::filesystem::path file = dir / timestampName();
        if (saveScreenshot(file)) log::info("Screenshot saved: {}", file.string());
    }
}

void App::rebuildCloudIfNeeded(bool force) {
    if (!force && cloud_.seed() == config_.cloudSeed && cloud_.resolution() > 0) return;
    const auto start = std::chrono::steady_clock::now();
    cloud_ = CloudField::build(config_.cloudSeed);
    renderer_->setCloud(cloud_);
    log::info("Cloud rebuilt in {:.0f} ms",
              std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count());
}

void App::renderScene() {
    rebuildCloudIfNeeded(false);
    int fbw = 0, fbh = 0;
    glfwGetFramebufferSize(window_, &fbw, &fbh);
    if (fbw <= 0 || fbh <= 0) return;  // minimized
    renderer_->resize(fbw, fbh);

    scene_ = evaluateScene(playback_.time(), config_);
    const OrbitParams orbit = authoredOrbit(playback_.time(), scene_.pathway, visualStarRadius(scene_.ms.radiusSolar));
    const CameraPose pose = poseFromOrbit(orbit, cameraOffset_);
    renderer_->render(scene_, pose, lensShift_);

    ui_.beginFrame();
    UiFrameInfo info;
    info.scene = &scene_;
    info.fps = fps_;
    info.gpuMs = renderer_->gpuFrameMs();
    info.cameraOffsetActive = !cameraOffset_.isIdentity();
    info.project = [this](const glm::vec3& p) { return renderer_->projectToScreen(p); };
    info.quality = renderer_->quality();
    const UiRequests req = ui_.draw(playback_, config_, info);
    ui_.endFrame();

    if (req.recenterCamera) cameraOffset_ = {};
    if (req.quality) renderer_->setQuality(*req.quality);
    if (req.resetAll) resetAll();
}

void App::resetAll() {
    playback_.reset();
    cameraOffset_ = {};
}

bool App::saveScreenshot(const std::filesystem::path& path) {
    int w = 0, h = 0;
    glfwGetFramebufferSize(window_, &w, &h);
    std::vector<uint8_t> pixels(static_cast<size_t>(w) * h * 3);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadBuffer(GL_BACK);
    glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());
    std::vector<uint8_t> flipped(pixels.size());
    const size_t stride = static_cast<size_t>(w) * 3;
    for (int y = 0; y < h; ++y)
        std::memcpy(flipped.data() + y * stride, pixels.data() + (h - 1 - y) * stride, stride);
    if (!writePng(path, w, h, flipped)) {
        log::error("Could not write screenshot {}", path.string());
        return false;
    }
    return true;
}

int App::runCapture() {
    std::error_code ec;
    std::filesystem::create_directories(options_.captureDir, ec);
    int failures = 0;
    for (double t : options_.captureTimes) {
        playback_.seek(t);
        // Several frames so the GPU timer and composition shift settle.
        for (int i = 0; i < 4; ++i) {
            glfwPollEvents();
            frame(1.0 / 60.0);
            if (i < 3) glfwSwapBuffers(window_);
        }
        char name[64];
        std::snprintf(name, sizeof(name), "t%06.2f.png", t);
        const std::filesystem::path file = options_.captureDir / name;
        if (saveScreenshot(file)) {
            log::info("Captured t={:.2f}s -> {} (GPU {:.2f} ms)", t, file.string(), renderer_->gpuFrameMs());
        } else {
            ++failures;
        }
        glfwSwapBuffers(window_);
    }
    return failures == 0 ? 0 : 1;
}

int App::runSelfTest() {
    // Exercises the interaction paths against the real renderer. Pure timeline
    // logic is covered by the unit tests; this checks that rendering stays
    // healthy (no GL errors) through seeks, stage jumps, playback and resets.
    int failures = 0;
    auto check = [&](bool ok, const char* what) {
        if (!ok) { log::error("SELFTEST FAIL: {}", what); ++failures; }
        else log::info("selftest ok: {}", what);
    };
    auto renderFrames = [&](int n, double dt) {
        for (int i = 0; i < n; ++i) { glfwPollEvents(); frame(dt); glfwSwapBuffers(window_); }
    };
    auto glClean = [] {
        bool clean = true;
        for (GLenum e = glGetError(); e != GL_NO_ERROR; e = glGetError()) {
            log::error("glGetError 0x{:x}", e);
            clean = false;
        }
        return clean;
    };

    renderFrames(3, 1.0 / 60.0);
    check(glClean(), "initial frames render without GL errors");

    for (int s = 0; s < kStageCount; ++s) {
        playback_.selectStage(static_cast<Stage>(s));
        renderFrames(2, 1.0 / 60.0);
        check(scene_.stage == static_cast<Stage>(s), "stage selection produces the selected stage");
    }
    check(glClean(), "all stages render without GL errors");

    for (double m : {1.0, 15.0, 40.0}) {
        config_.massSolar = m;
        playback_.seek(timelineDuration() * 0.9);
        renderFrames(2, 1.0 / 60.0);
    }
    config_ = options_.config;
    check(glClean(), "all three final-stage pathways render without GL errors");

    playback_.seek(30.0);
    playback_.play();
    renderFrames(30, 1.0 / 60.0);
    check(playback_.time() > 30.0 && playback_.time() < 31.5, "playback advances at 1x");
    playback_.pause();
    const double paused = playback_.time();
    renderFrames(10, 1.0 / 60.0);
    check(playback_.time() == paused, "pause holds the timeline position");

    // Rapid back-and-forth seeking across stage boundaries.
    for (int i = 0; i < 40; ++i) {
        playback_.seek((i % 2 == 0) ? 17.9 + i : 120.0 - i);
        renderFrames(1, 1.0 / 60.0);
    }
    check(glClean(), "rapid seeking renders without GL errors");

    resetAll();
    renderFrames(2, 1.0 / 60.0);
    check(playback_.time() == 0.0 && !playback_.playing() && playback_.speed() == 1.0, "reset restores the initial state");

    glfwSetWindowSize(window_, 1280, 720);
    renderFrames(3, 1.0 / 60.0);
    check(glClean(), "window resize recreates targets without GL errors");

    log::info("Self-test finished: {} failure(s). GPU frame ~{:.2f} ms", failures, renderer_->gpuFrameMs());
    return failures == 0 ? 0 : 1;
}

int App::runBenchmark() {
    // Measured with vsync off so the numbers reflect rendering cost. GPU time comes
    // from GL_TIME_ELAPSED queries (smoothed); frame time is wall-clock per frame.
    glfwSwapInterval(0);
    struct Sample { const char* label; double time; double mass; };
    const Sample samples[] = {
        {"1 cloud", 8, 1}, {"2 collapse", 32, 1}, {"3 protostar", 52, 1}, {"4 disc+jets", 72, 1},
        {"5 main seq", 94, 1}, {"6 WD nebula", 140, 1}, {"6 supernova", 116, 15}, {"6 NS remnant", 140, 15},
        {"6 black hole", 147, 40}};
    int fbw = 0, fbh = 0;
    glfwGetFramebufferSize(window_, &fbw, &fbh);
    log::info("Benchmark at {}x{} (vsync off):", fbw, fbh);
    double worstGpu = 0.0;
    for (const Sample& s : samples) {
        config_.massSolar = s.mass;
        playback_.seek(s.time);
        for (int i = 0; i < 30; ++i) { glfwPollEvents(); frame(1.0 / 60.0); glfwSwapBuffers(window_); }  // warm-up
        double total = 0.0, worst = 0.0;
        const int frames = 150;
        for (int i = 0; i < frames; ++i) {
            const auto start = std::chrono::steady_clock::now();
            glfwPollEvents();
            frame(0.0);
            glfwSwapBuffers(window_);
            const double ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count();
            total += ms;
            worst = std::max(worst, ms);
        }
        worstGpu = std::max(worstGpu, static_cast<double>(renderer_->gpuFrameMs()));
        log::info("  {:<14} GPU {:5.2f} ms | frame avg {:5.2f} ms ({:4.0f} fps), worst {:5.2f} ms", s.label,
                  renderer_->gpuFrameMs(), total / frames, 1000.0 * frames / total, worst);
    }
    config_ = options_.config;
    log::info("Benchmark done. Worst stage GPU time {:.2f} ms.", worstGpu);
    return 0;
}

void App::toggleFullscreen() {
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    if (glfwGetWindowMonitor(window_)) {
        glfwSetWindowMonitor(window_, nullptr, windowedPos_[0], windowedPos_[1], windowedSize_[0], windowedSize_[1], 0);
    } else if (monitor) {
        glfwGetWindowPos(window_, &windowedPos_[0], &windowedPos_[1]);
        glfwGetWindowSize(window_, &windowedSize_[0], &windowedSize_[1]);
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        glfwSetWindowMonitor(window_, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
    }
}

// --- Input ---------------------------------------------------------------------

void App::onKey(GLFWwindow* w, int key, int /*scancode*/, int action, int mods) {
    auto* app = static_cast<App*>(glfwGetWindowUserPointer(w));
    if (!app || !app->uiReady_ || app->ui_.wantsKeyboard()) return;
    if (action != GLFW_PRESS && action != GLFW_REPEAT) return;
    Playback& pb = app->playback_;
    const double step = (mods & GLFW_MOD_SHIFT) ? 1.0 : 5.0;
    switch (key) {
    case GLFW_KEY_SPACE: if (action == GLFW_PRESS) pb.togglePlay(); break;
    case GLFW_KEY_LEFT:  pb.step(-step); break;
    case GLFW_KEY_RIGHT: pb.step(step); break;
    case GLFW_KEY_UP:    if (action == GLFW_PRESS) pb.faster(); break;
    case GLFW_KEY_DOWN:  if (action == GLFW_PRESS) pb.slower(); break;
    case GLFW_KEY_HOME:
    case GLFW_KEY_R:     if (action == GLFW_PRESS) app->resetAll(); break;
    case GLFW_KEY_C:     app->cameraOffset_ = {}; break;
    case GLFW_KEY_H:     if (action == GLFW_PRESS) app->ui_.toggleVisible(); break;
    case GLFW_KEY_F5:    if (action == GLFW_PRESS) app->renderer_->reloadShaders(); break;
    case GLFW_KEY_F11:   if (action == GLFW_PRESS) app->toggleFullscreen(); break;
    case GLFW_KEY_F12:   if (action == GLFW_PRESS) app->screenshotRequested_ = true; break;
    case GLFW_KEY_ESCAPE:
        if (glfwGetWindowMonitor(w)) app->toggleFullscreen();
        break;
    default:
        if (key >= GLFW_KEY_1 && key <= GLFW_KEY_6 && action == GLFW_PRESS)
            pb.selectStage(static_cast<Stage>(key - GLFW_KEY_1));
        break;
    }
}

void App::onMouseButton(GLFWwindow* w, int button, int action, int /*mods*/) {
    auto* app = static_cast<App*>(glfwGetWindowUserPointer(w));
    if (!app || !app->uiReady_ || button != GLFW_MOUSE_BUTTON_LEFT) return;
    if (action == GLFW_PRESS) {
        if (app->ui_.wantsMouse()) return;
        const double now = glfwGetTime();
        if (app->lastClickTime_ > 0.0 && now - app->lastClickTime_ < 0.3) app->cameraOffset_ = {};  // double-click
        app->lastClickTime_ = now;
        app->dragging_ = true;
        glfwGetCursorPos(w, &app->lastX_, &app->lastY_);
    } else if (action == GLFW_RELEASE) {
        app->dragging_ = false;
    }
}

void App::onCursorPos(GLFWwindow* w, double x, double y) {
    auto* app = static_cast<App*>(glfwGetWindowUserPointer(w));
    if (!app || !app->dragging_) return;
    const double dx = x - app->lastX_, dy = y - app->lastY_;
    app->lastX_ = x;
    app->lastY_ = y;
    app->cameraOffset_.yawDeg -= static_cast<float>(dx) * 0.25f;
    app->cameraOffset_.pitchDeg = std::clamp(app->cameraOffset_.pitchDeg + static_cast<float>(dy) * 0.2f, -70.0f, 70.0f);
}

void App::onScroll(GLFWwindow* w, double /*dx*/, double dy) {
    auto* app = static_cast<App*>(glfwGetWindowUserPointer(w));
    if (!app || !app->uiReady_ || app->ui_.wantsMouse()) return;
    app->cameraOffset_.zoom = std::clamp(app->cameraOffset_.zoom * static_cast<float>(std::pow(0.9, dy)), 0.3f, 3.0f);
}

} // namespace stellaris
