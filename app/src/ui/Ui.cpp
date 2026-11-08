#include "ui/Ui.h"

#include "core/Log.h"
#include "sim/StageContent.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <string>

namespace stellaris {

namespace {

// Palette (linear-ish UI colors, ImGui expects sRGB-encoded values).
const ImVec4 kText      = ImVec4(0.90f, 0.92f, 0.96f, 1.00f);
const ImVec4 kTextDim   = ImVec4(0.62f, 0.66f, 0.76f, 1.00f);
const ImVec4 kTextFaint = ImVec4(0.45f, 0.49f, 0.60f, 1.00f);
const ImVec4 kAccent    = ImVec4(0.98f, 0.76f, 0.42f, 1.00f);
const ImVec4 kPanelBg   = ImVec4(0.025f, 0.03f, 0.06f, 0.62f);

ImU32 col32(const ImVec4& c, float alphaMul = 1.0f) {
    return ImGui::ColorConvertFloat4ToU32(ImVec4(c.x, c.y, c.z, c.w * alphaMul));
}

// Color identity of each stage on the timeline.
ImVec4 stageColor(Stage s, Pathway p) {
    switch (s) {
    case Stage::MolecularCloud: return ImVec4(0.42f, 0.46f, 0.85f, 1.0f);
    case Stage::Collapse:       return ImVec4(0.62f, 0.42f, 0.82f, 1.0f);
    case Stage::Protostar:      return ImVec4(0.95f, 0.55f, 0.30f, 1.0f);
    case Stage::DiscAndJets:    return ImVec4(0.98f, 0.70f, 0.38f, 1.0f);
    case Stage::MainSequence:   return ImVec4(1.00f, 0.88f, 0.62f, 1.0f);
    case Stage::StellarFate:
        return p == Pathway::WhiteDwarf ? ImVec4(0.55f, 0.85f, 0.90f, 1.0f)
             : p == Pathway::NeutronStar ? ImVec4(0.60f, 0.72f, 1.00f, 1.0f)
                                         : ImVec4(0.78f, 0.55f, 0.95f, 1.0f);
    }
    return kText;
}

std::string formatTime(double seconds) {
    const int s = static_cast<int>(seconds);
    char buf[16];
    std::snprintf(buf, sizeof(buf), "%d:%02d", s / 60, s % 60);
    return buf;
}

// "3.1×10⁵" style scientific notation with Unicode superscripts.
std::string sci(double v, int digits = 1) {
    if (v <= 0.0) return "0";
    int e = static_cast<int>(std::floor(std::log10(v)));
    double m = v / std::pow(10.0, e);
    if (m >= 9.95) { m /= 10.0; ++e; }
    if (e >= -1 && e <= 3) {
        char buf[32];
        std::snprintf(buf, sizeof(buf), "%.*f", e >= 2 ? 0 : digits, v);
        return buf;
    }
    static const char* sup[] = {"\u2070", "\u00b9", "\u00b2", "\u00b3", "\u2074", "\u2075", "\u2076", "\u2077", "\u2078", "\u2079"};
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%.*f\u00d710", digits, m);
    std::string out = buf;
    if (e < 0) { out += "\u207b"; e = -e; }
    const std::string digitsStr = std::to_string(e);
    for (char c : digitsStr) out += sup[c - '0'];
    return out;
}

// One row of the quantities table: label, value, provenance tag.
void quantityRow(const char* label, const std::string& value, const char* tag) {
    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::PushStyleColor(ImGuiCol_Text, kTextDim);
    ImGui::TextUnformatted(label);
    ImGui::PopStyleColor();
    ImGui::TableSetColumnIndex(1);
    ImGui::TextUnformatted(value.c_str());
    ImGui::TableSetColumnIndex(2);
    ImGui::PushStyleColor(ImGuiCol_Text, kTextFaint);
    ImGui::TextUnformatted(tag);
    ImGui::PopStyleColor();
}

std::filesystem::path windowsFontDir() {
    const char* windir = std::getenv("WINDIR");
    return std::filesystem::path(windir ? windir : "C:\\Windows") / "Fonts";
}

// Small vector icons for the transport buttons (font independent).
enum class Icon { Play, Pause, Back, Forward, Reset };

bool iconButton(const char* id, Icon icon, float size, const char* tooltip) {
    const ImVec2 p = ImGui::GetCursorScreenPos();
    const bool pressed = ImGui::InvisibleButton(id, ImVec2(size, size));
    const bool hovered = ImGui::IsItemHovered();
    ImDrawList* dl = ImGui::GetWindowDrawList();
    const ImVec2 c(p.x + size * 0.5f, p.y + size * 0.5f);
    dl->AddCircleFilled(c, size * 0.5f, col32(ImVec4(1, 1, 1, hovered ? 0.14f : 0.06f)), 32);
    const ImU32 fg = col32(hovered ? kAccent : kText);
    const float r = size * 0.22f;
    switch (icon) {
    case Icon::Play:
        dl->AddTriangleFilled(ImVec2(c.x - r * 0.7f, c.y - r), ImVec2(c.x - r * 0.7f, c.y + r), ImVec2(c.x + r * 1.1f, c.y), fg);
        break;
    case Icon::Pause:
        dl->AddRectFilled(ImVec2(c.x - r * 0.8f, c.y - r), ImVec2(c.x - r * 0.25f, c.y + r), fg);
        dl->AddRectFilled(ImVec2(c.x + r * 0.25f, c.y - r), ImVec2(c.x + r * 0.8f, c.y + r), fg);
        break;
    case Icon::Back:
        dl->AddTriangleFilled(ImVec2(c.x + r * 0.9f, c.y - r * 0.8f), ImVec2(c.x + r * 0.9f, c.y + r * 0.8f), ImVec2(c.x, c.y), fg);
        dl->AddTriangleFilled(ImVec2(c.x, c.y - r * 0.8f), ImVec2(c.x, c.y + r * 0.8f), ImVec2(c.x - r * 0.9f, c.y), fg);
        break;
    case Icon::Forward:
        dl->AddTriangleFilled(ImVec2(c.x - r * 0.9f, c.y - r * 0.8f), ImVec2(c.x - r * 0.9f, c.y + r * 0.8f), ImVec2(c.x, c.y), fg);
        dl->AddTriangleFilled(ImVec2(c.x, c.y - r * 0.8f), ImVec2(c.x, c.y + r * 0.8f), ImVec2(c.x + r * 0.9f, c.y), fg);
        break;
    case Icon::Reset:
        dl->PathArcTo(c, r, 0.6f, 5.6f, 20);
        dl->PathStroke(fg, 0, size * 0.07f);
        dl->AddTriangleFilled(ImVec2(c.x + r * 0.55f, c.y - r * 0.95f), ImVec2(c.x + r * 1.25f, c.y - r * 0.15f),
                              ImVec2(c.x + r * 1.15f, c.y - r * 1.15f), fg);
        break;
    }
    if (hovered && tooltip) ImGui::SetTooltip("%s", tooltip);
    return pressed;
}

} // namespace

bool Ui::init(GLFWwindow* window, float contentScale) {
    baseScale_ = scale_ = std::clamp(contentScale, 1.0f, 3.0f);
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;  // no imgui.ini: the layout is fixed and deterministic

    // Fonts: Segoe UI family ships with Windows; fall back to ImGui's default.
    ImFontGlyphRangesBuilder builder;
    builder.AddRanges(io.Fonts->GetGlyphRangesDefault());
    builder.AddText("\u2609\u2248\u221d\u00d7\u2013\u2014\u00b7\u2264\u2265\u2192\u03c9\u03c1\u03b2\u03b8\u2081\u2082\u2080");
    builder.AddText("\u2070\u00b9\u00b2\u00b3\u2074\u2075\u2076\u2077\u2078\u2079\u207b\u201c\u201d\u2019");
    static ImVector<ImWchar> ranges;
    builder.BuildRanges(&ranges);

    const std::filesystem::path fonts = windowsFontDir();
    auto load = [&](const char* file, float px) -> ImFont* {
        const std::filesystem::path p = fonts / file;
        if (!std::filesystem::exists(p)) return nullptr;
        ImFontConfig cfg;
        cfg.OversampleH = 2;
        ImFont* f = io.Fonts->AddFontFromFileTTF(p.string().c_str(), px * scale_, &cfg, ranges.Data);
        // Merge symbol glyphs (e.g. the solar symbol) that Segoe UI lacks.
        const std::filesystem::path sym = fonts / "seguisym.ttf";
        if (f && std::filesystem::exists(sym)) {
            ImFontConfig merge;
            merge.MergeMode = true;
            static const ImWchar symRanges[] = {0x2190, 0x22FF, 0x2600, 0x26FF, 0};
            io.Fonts->AddFontFromFileTTF(sym.string().c_str(), px * scale_, &merge, symRanges);
        }
        return f;
    };
    body_    = load("segoeui.ttf", 16.0f);
    small_   = load("segoeui.ttf", 13.5f);
    heading_ = load("seguisb.ttf", 27.0f);
    title_   = load("segoeuisl.ttf", 14.0f);
    if (!body_) {
        log::warn("Segoe UI not found; using the built-in ImGui font.");
        ImFontConfig cfg;
        cfg.SizePixels = 15.0f * scale_;
        body_ = small_ = heading_ = title_ = io.Fonts->AddFontDefault(&cfg);
    }
    if (!small_) small_ = body_;
    if (!heading_) heading_ = body_;
    if (!title_) title_ = small_;
    io.FontDefault = body_;

    ImGuiStyle& st = ImGui::GetStyle();
    st.WindowRounding = 8.0f;
    st.FrameRounding = 5.0f;
    st.GrabRounding = 5.0f;
    st.WindowBorderSize = 0.0f;
    st.FrameBorderSize = 0.0f;
    st.WindowPadding = ImVec2(16, 14);
    st.ItemSpacing = ImVec2(8, 7);
    st.ScaleAllSizes(scale_);
    ImVec4* c = st.Colors;
    c[ImGuiCol_Text] = kText;
    c[ImGuiCol_TextDisabled] = kTextFaint;
    c[ImGuiCol_WindowBg] = kPanelBg;
    c[ImGuiCol_PopupBg] = ImVec4(0.04f, 0.05f, 0.09f, 0.95f);
    c[ImGuiCol_FrameBg] = ImVec4(1, 1, 1, 0.07f);
    c[ImGuiCol_FrameBgHovered] = ImVec4(1, 1, 1, 0.12f);
    c[ImGuiCol_FrameBgActive] = ImVec4(1, 1, 1, 0.16f);
    c[ImGuiCol_SliderGrab] = kAccent;
    c[ImGuiCol_SliderGrabActive] = ImVec4(1.0f, 0.85f, 0.6f, 1.0f);
    c[ImGuiCol_Button] = ImVec4(1, 1, 1, 0.07f);
    c[ImGuiCol_ButtonHovered] = ImVec4(1, 1, 1, 0.14f);
    c[ImGuiCol_ButtonActive] = ImVec4(0.98f, 0.76f, 0.42f, 0.45f);
    c[ImGuiCol_CheckMark] = kAccent;
    c[ImGuiCol_Separator] = ImVec4(1, 1, 1, 0.10f);
    c[ImGuiCol_TableRowBg] = ImVec4(0, 0, 0, 0);
    c[ImGuiCol_TableRowBgAlt] = ImVec4(1, 1, 1, 0.025f);

    if (!ImGui_ImplGlfw_InitForOpenGL(window, true) || !ImGui_ImplOpenGL3_Init("#version 450")) {
        log::error("Failed to initialize the Dear ImGui backends.");
        return false;
    }
    initialized_ = true;
    return true;
}

void Ui::shutdown() {
    if (!initialized_) return;
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    initialized_ = false;
}

void Ui::beginFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    // The layout is designed for 1600x900 logical pixels; shrink it for smaller
    // windows (with a floor that keeps text readable) so panels leave room for the scene.
    ImGuiIO& io = ImGui::GetIO();
    const float logicalW = io.DisplaySize.x / baseScale_, logicalH = io.DisplaySize.y / baseScale_;
    layout_ = std::clamp(std::min(logicalW / 1600.0f, logicalH / 900.0f), 0.72f, 1.0f);
    scale_ = baseScale_ * layout_;
    io.FontGlobalScale = layout_;
    ImGui::NewFrame();
}

void Ui::endFrame() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

bool Ui::wantsMouse() const { return ImGui::GetIO().WantCaptureMouse; }
bool Ui::wantsKeyboard() const { return ImGui::GetIO().WantCaptureKeyboard; }

UiRequests Ui::draw(Playback& playback, ModelConfig& config, const UiFrameInfo& info) {
    UiRequests req;
    const SceneState& s = *info.scene;
    if (labels_) drawAnnotations(info);
    if (!visible_) {
        // Minimal hint so the presenter knows how to bring the interface back.
        ImDrawList* dl = ImGui::GetForegroundDrawList();
        const ImVec2 ds = ImGui::GetIO().DisplaySize;
        dl->AddText(small_, small_->FontSize * layout_, ImVec2(16 * scale_, ds.y - 28 * scale_), col32(kTextFaint, 0.7f),
                    "H  show interface");
        return req;
    }
    drawInfoPanel(s);
    drawParameterPanel(config, s, req, info.cameraOffsetActive);
    drawTimeline(playback, s, req);
    drawDiagnostics(info);
    return req;
}

void Ui::drawInfoPanel(const SceneState& s) {
    const float w = 390.0f * scale_;
    const float m = 18.0f * scale_;
    ImGui::SetNextWindowPos(ImVec2(m, m));
    ImGui::SetNextWindowSize(ImVec2(w, 0));
    ImGui::Begin("##info", nullptr,
                 ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings |
                     ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoFocusOnAppearing);

    const int idx = static_cast<int>(s.stage);
    const StageContent content = stageContent(s.stage, s.pathway);

    ImGui::PushFont(title_);
    ImGui::PushStyleColor(ImGuiCol_Text, kTextFaint);
    ImGui::Text("S T E L L A R I S     \u00b7     STAGE %d OF %d", idx + 1, kStageCount);
    ImGui::PopStyleColor();
    ImGui::PopFont();

    ImGui::PushFont(heading_);
    ImGui::PushStyleColor(ImGuiCol_Text, stageColor(s.stage, s.pathway));
    ImGui::TextWrapped("%s", stageSpan(s.stage).name);
    ImGui::PopStyleColor();
    ImGui::PopFont();

    ImGui::PushStyleColor(ImGuiCol_Text, kAccent);
    ImGui::TextWrapped("%s", content.process);
    ImGui::PopStyleColor();
    ImGui::Spacing();
    ImGui::TextWrapped("%s", content.explanation);
    ImGui::Spacing();

    ImGui::PushFont(small_);
    ImGui::PushStyleColor(ImGuiCol_Text, kTextDim);
    ImGui::TextUnformatted("IN THIS VISUALIZATION");
    ImGui::PopStyleColor();
    ImGui::PushStyleColor(ImGuiCol_Text, kTextDim);
    ImGui::TextWrapped("%s", content.approximation);
    ImGui::PopStyleColor();
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    if (ImGui::BeginTable("##quantities", 3, ImGuiTableFlags_SizingStretchProp)) {
        ImGui::TableSetupColumn("q", ImGuiTableColumnFlags_WidthStretch, 1.5f);
        ImGui::TableSetupColumn("v", ImGuiTableColumnFlags_WidthStretch, 1.0f);
        ImGui::TableSetupColumn("t", ImGuiTableColumnFlags_WidthStretch, 0.85f);
        char buf[64];
        switch (s.stage) {
        case Stage::MolecularCloud:
            quantityRow("Typical cloud temperature", "10\u201320 K", "reference");
            quantityRow("Cloud structure", "procedural", "illustrative");
            break;
        case Stage::Collapse:
            std::snprintf(buf, sizeof(buf), "%.2f", s.collapse);
            quantityRow("Collapse progress", buf, "prescribed, 0\u20131");
            quantityRow("Free-fall time, n(H\u2082)=10\u2074 cm\u207b\u00b3", "\u2248 " + sci(freeFallTimeYears(1e4)) + " yr", "calculated");
            break;
        case Stage::Protostar:
            std::snprintf(buf, sizeof(buf), "\u2248 %.0f K", s.centralTempK);
            quantityRow("Color temperature shown", buf, "illustrative");
            quantityRow("Energy source", "contraction, accretion", "reference");
            break;
        case Stage::DiscAndJets:
            std::snprintf(buf, sizeof(buf), "%.2f\u00d7", s.rotation);
            quantityRow("Initial core rotation", buf, "parameter");
            std::snprintf(buf, sizeof(buf), "%.2f\u00d7", s.rotation * s.rotation);
            quantityRow("Disc size (\u221d rotation\u00b2)", buf, "model relation");
            quantityRow("Disc temperature profile", "T \u221d r^-0.75", "model relation");
            break;
        case Stage::MainSequence:
        case Stage::StellarFate:
            std::snprintf(buf, sizeof(buf), "%.1f M\u2609", s.ms.massSolar);
            quantityRow("Initial mass", buf, "parameter");
            quantityRow("Luminosity", sci(s.ms.luminositySolar, 1) + " L\u2609", "calculated");
            std::snprintf(buf, sizeof(buf), "%.2f R\u2609", s.ms.radiusSolar);
            quantityRow("Radius", buf, "calculated");
            std::snprintf(buf, sizeof(buf), "%.0f K", std::round(s.ms.teffK / 100.0) * 100.0);
            quantityRow("Surface temperature", buf, "calculated");
            quantityRow("Main-sequence lifetime", "~" + sci(s.ms.lifetimeYears, 0) + " yr", "order of mag.");
            if (s.stage == Stage::StellarFate) {
                quantityRow("Pathway (illustrative)", pathwayName(s.pathway), "category");
                quantityRow("Remnant", remnantName(s.pathway), "category");
            }
            break;
        }
        ImGui::EndTable();
    }
    ImGui::PopFont();
    ImGui::End();
}

void Ui::drawParameterPanel(ModelConfig& config, const SceneState& s, UiRequests& req, bool cameraOffset) {
    const float w = 300.0f * scale_;
    const float m = 18.0f * scale_;
    const ImVec2 ds = ImGui::GetIO().DisplaySize;
    ImGui::SetNextWindowPos(ImVec2(ds.x - w - m, m));
    ImGui::SetNextWindowSize(ImVec2(w, 0));
    ImGui::Begin("##params", nullptr,
                 ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings |
                     ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoFocusOnAppearing);
    ImGui::PushFont(small_);
    ImGui::PushStyleColor(ImGuiCol_Text, kTextFaint);
    ImGui::TextUnformatted("MODEL PARAMETERS");
    ImGui::PopStyleColor();

    ImGui::TextUnformatted("Initial stellar mass");
    float mass = static_cast<float>(config.massSolar);
    ImGui::SetNextItemWidth(-1);
    if (ImGui::SliderFloat("##mass", &mass, static_cast<float>(kMinMassSolar), static_cast<float>(kMaxMassSolar),
                           "%.1f M\u2609", ImGuiSliderFlags_Logarithmic | ImGuiSliderFlags_AlwaysClamp))
        config.massSolar = mass;
    const float bw = (ImGui::GetContentRegionAvail().x - 2 * ImGui::GetStyle().ItemSpacing.x) / 3.0f;
    if (ImGui::Button("1 M\u2609", ImVec2(bw, 0))) config.massSolar = 1.0;
    ImGui::SameLine();
    if (ImGui::Button("15 M\u2609", ImVec2(bw, 0))) config.massSolar = 15.0;
    ImGui::SameLine();
    if (ImGui::Button("40 M\u2609", ImVec2(bw, 0))) config.massSolar = 40.0;

    ImGui::PushStyleColor(ImGuiCol_Text, stageColor(Stage::StellarFate, s.pathway));
    ImGui::Text("\u2192 %s  \u00b7  %s", pathwayName(s.pathway), remnantName(s.pathway));
    ImGui::PopStyleColor();
    if (massNearCategoryBoundary(config.massSolar)) {
        ImGui::PushStyleColor(ImGuiCol_Text, kAccent);
        ImGui::TextWrapped("Near an uncertain boundary: real outcome also depends on composition, rotation, "
                           "mass loss and companions.");
        ImGui::PopStyleColor();
    }

    ImGui::Spacing();
    ImGui::TextUnformatted("Initial core rotation");
    float rot = static_cast<float>(config.rotation);
    ImGui::SetNextItemWidth(-1);
    if (ImGui::SliderFloat("##rot", &rot, static_cast<float>(ModelConfig::kMinRotation),
                           static_cast<float>(ModelConfig::kMaxRotation), "%.2f\u00d7", ImGuiSliderFlags_AlwaysClamp))
        config.rotation = rot;
    ImGui::PushStyleColor(ImGuiCol_Text, kTextFaint);
    ImGui::TextWrapped("Sets the gas's angular momentum: faster rotation gives a larger disc.");
    ImGui::PopStyleColor();

    ImGui::Spacing();
    ImGui::TextUnformatted("Cloud structure");
    ImGui::SameLine();
    if (ImGui::SmallButton("<##seed") && config.cloudSeed > 1) --config.cloudSeed;
    ImGui::SameLine();
    ImGui::Text("seed %u", config.cloudSeed);
    ImGui::SameLine();
    if (ImGui::SmallButton(">##seed")) ++config.cloudSeed;
    ImGui::PushStyleColor(ImGuiCol_Text, kTextFaint);
    ImGui::TextWrapped("Procedural hub-filament cloud (illustrative, not a measured region).");
    ImGui::PopStyleColor();

    ImGui::Spacing();
    ImGui::Checkbox("3D labels", &labels_);
    ImGui::SameLine();
    if (ImGui::Button(cameraOffset ? "Recenter camera *" : "Recenter camera")) req.recenterCamera = true;
    if (ImGui::Button("Default parameters")) {
        config = ModelConfig{};
    }
    ImGui::PushStyleColor(ImGuiCol_Text, kTextFaint);
    ImGui::TextWrapped("Drag: orbit  \u00b7  Wheel: zoom  \u00b7  Space: play  \u00b7  \u2190/\u2192: seek  "
                       "\u00b7  1\u20136: stage  \u00b7  H: hide UI  \u00b7  F12: screenshot");
    ImGui::PopStyleColor();
    ImGui::PopFont();
    ImGui::End();
}

void Ui::drawTimeline(Playback& playback, const SceneState& s, UiRequests& req) {
    const ImVec2 ds = ImGui::GetIO().DisplaySize;
    const float m = 18.0f * scale_;
    const float h = 104.0f * scale_;
    ImGui::SetNextWindowPos(ImVec2(m, ds.y - h - m));
    ImGui::SetNextWindowSize(ImVec2(ds.x - 2 * m, h));
    ImGui::Begin("##timeline", nullptr,
                 ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings |
                     ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoFocusOnAppearing);

    // Transport row.
    const float btn = 30.0f * scale_;
    if (iconButton("##reset", Icon::Reset, btn, "Reset to the beginning (R)")) req.resetAll = true;
    ImGui::SameLine();
    if (iconButton("##back", Icon::Back, btn, "Back 5 s (Left)")) playback.step(-5.0);
    ImGui::SameLine();
    if (iconButton("##play", playback.playing() ? Icon::Pause : Icon::Play, btn, "Play / pause (Space)"))
        playback.togglePlay();
    ImGui::SameLine();
    if (iconButton("##fwd", Icon::Forward, btn, "Forward 5 s (Right)")) playback.step(5.0);
    ImGui::SameLine(0, 18.0f * scale_);

    ImGui::PushFont(small_);
    // SameLine() restores the row's original Y, so center every item explicitly.
    const float rowY = ImGui::GetCursorPosY() + (btn - ImGui::GetFrameHeight()) * 0.5f;
    for (int i = 0; i < static_cast<int>(Playback::kSpeeds.size()); ++i) {
        ImGui::SetCursorPosY(rowY);
        char label[16];
        std::snprintf(label, sizeof(label), "%g\u00d7##sp%d", Playback::kSpeeds[i], i);
        const bool active = i == playback.speedIndex();
        if (active) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.98f, 0.76f, 0.42f, 0.35f));
        if (ImGui::SmallButton(label)) playback.setSpeedIndex(i);
        if (active) ImGui::PopStyleColor();
        ImGui::SameLine();
    }
    ImGui::SameLine(0, 18.0f * scale_);
    ImGui::SetCursorPosY(rowY + ImGui::GetStyle().FramePadding.y);
    const std::string clock = formatTime(playback.time()) + " / " + formatTime(timelineDuration());
    ImGui::TextUnformatted(clock.c_str());
    ImGui::SameLine();
    const char* note = "Narrative timeline \u2014 not to physical time scale";
    const float noteW = ImGui::CalcTextSize(note).x;
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + std::max(0.0f, ImGui::GetContentRegionAvail().x - noteW));
    ImGui::SetCursorPosY(rowY + ImGui::GetStyle().FramePadding.y);
    ImGui::PushStyleColor(ImGuiCol_Text, kTextFaint);
    ImGui::TextUnformatted(note);
    ImGui::PopStyleColor();

    // Track with stage segments.
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2.0f * scale_);
    const ImVec2 p0 = ImGui::GetCursorScreenPos();
    const float width = ImGui::GetContentRegionAvail().x;
    const float trackH = 8.0f * scale_;
    const float labelH = ImGui::GetFontSize() + 6.0f * scale_;
    ImGui::InvisibleButton("##track", ImVec2(width, trackH + labelH + 6.0f * scale_));
    const bool hovered = ImGui::IsItemHovered();
    const double duration = timelineDuration();
    const float mouseX = ImGui::GetIO().MousePos.x;
    const double hoverT = std::clamp((mouseX - p0.x) / width, 0.0f, 1.0f) * duration;
    // Clicking a stage label selects that stage (and nothing else while the button
    // is held); clicking or dragging the track itself seeks continuously.
    if (ImGui::IsItemActivated())
        labelClick_ = ImGui::GetIO().MouseClickedPos[0].y > p0.y + trackH + 3.0f * scale_;
    if (ImGui::IsItemActive()) {
        if (!labelClick_) playback.seek(hoverT);
        else if (ImGui::IsItemActivated()) playback.selectStage(stageAt(hoverT));
    }

    ImDrawList* dl = ImGui::GetWindowDrawList();
    const float gap = 3.0f * scale_;
    for (const StageSpan& span : stageTable()) {
        const float x0 = p0.x + static_cast<float>(span.start / duration) * width;
        const float x1 = p0.x + static_cast<float>(span.end / duration) * width;
        const ImVec4 sc = stageColor(span.stage, s.pathway);
        const bool current = span.stage == s.stage;
        dl->AddRectFilled(ImVec2(x0 + gap * 0.5f, p0.y), ImVec2(x1 - gap * 0.5f, p0.y + trackH),
                          col32(sc, current ? 0.40f : 0.16f), trackH * 0.5f);
        // Filled portion up to the playhead.
        const float fillX = std::clamp(p0.x + static_cast<float>(playback.time() / duration) * width, x0, x1);
        if (fillX > x0 + gap * 0.5f)
            dl->AddRectFilled(ImVec2(x0 + gap * 0.5f, p0.y), ImVec2(std::min(fillX, x1 - gap * 0.5f), p0.y + trackH),
                              col32(sc, 0.85f), trackH * 0.5f);
        const ImVec2 ts = ImGui::CalcTextSize(span.shortName);
        const float tx = std::max(x0 + 4.0f * scale_, (x0 + x1 - ts.x) * 0.5f);
        dl->AddText(ImVec2(tx, p0.y + trackH + 5.0f * scale_), col32(current ? sc : kTextDim, current ? 1.0f : 0.8f),
                    span.shortName);
    }
    const float px = p0.x + static_cast<float>(playback.time() / duration) * width;
    dl->AddCircleFilled(ImVec2(px, p0.y + trackH * 0.5f), 7.0f * scale_, col32(ImVec4(1, 1, 1, 1)), 24);
    dl->AddCircle(ImVec2(px, p0.y + trackH * 0.5f), 7.0f * scale_, col32(kAccent), 24, 2.0f * scale_);
    if (hovered) {
        ImGui::SetTooltip("%s  \u00b7  %s", formatTime(hoverT).c_str(), stageSpan(stageAt(hoverT)).name);
    }
    ImGui::PopFont();
    ImGui::End();
}

void Ui::drawAnnotations(const UiFrameInfo& info) {
    const SceneState& s = *info.scene;
    if (!info.project) return;
    ImDrawList* dl = ImGui::GetBackgroundDrawList();
    const ImVec2 ds = ImGui::GetIO().DisplaySize;
    for (int i = 0; i < s.annotationCount; ++i) {
        const Annotation& a = s.annotations[i];
        const auto p = info.project(a.anchor);
        if (!p || p->x < 0 || p->y < 0 || p->x > ds.x || p->y > ds.y) continue;
        const ImVec2 anchor(p->x, p->y);
        const ImVec2 elbow(anchor.x + 46.0f * scale_, anchor.y - 34.0f * scale_);
        const ImVec2 end(elbow.x + 18.0f * scale_, elbow.y);
        const float alpha = a.alpha;
        dl->AddCircleFilled(anchor, 2.5f * scale_, col32(kText, 0.9f * alpha));
        dl->AddLine(anchor, elbow, col32(kText, 0.45f * alpha), 1.0f * scale_);
        dl->AddLine(elbow, end, col32(kText, 0.45f * alpha), 1.0f * scale_);
        const ImVec2 textPos(end.x + 6.0f * scale_, end.y - small_->FontSize * layout_ * 0.55f);
        const ImVec2 ts = small_->CalcTextSizeA(small_->FontSize * layout_, FLT_MAX, 0.0f, a.text);
        dl->AddRectFilled(ImVec2(textPos.x - 5 * scale_, textPos.y - 2 * scale_),
                          ImVec2(textPos.x + ts.x + 5 * scale_, textPos.y + ts.y + 2 * scale_),
                          col32(ImVec4(0.02f, 0.03f, 0.06f, 0.55f), alpha), 4.0f * scale_);
        dl->AddText(small_, small_->FontSize * layout_, textPos, col32(kText, alpha), a.text);
    }
}

void Ui::drawDiagnostics(const UiFrameInfo& info) {
    const ImVec2 ds = ImGui::GetIO().DisplaySize;
    char buf[64];
    std::snprintf(buf, sizeof(buf), "%.0f fps  \u00b7  GPU %.1f ms", info.fps, info.gpuMs);
    const ImVec2 ts = small_->CalcTextSizeA(small_->FontSize * layout_, FLT_MAX, 0.0f, buf);
    ImGui::GetForegroundDrawList()->AddText(small_, small_->FontSize * layout_,
                                            ImVec2(ds.x - ts.x - 22.0f * scale_, ds.y - 104.0f * scale_ - 18.0f * scale_ - ts.y - 6.0f * scale_),
                                            col32(kTextFaint, 0.8f), buf);
}

} // namespace stellaris
