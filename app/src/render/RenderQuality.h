#pragma once
// Render quality tiers (PROJECT_SPEC §18.13). They change rendering cost only:
// never scene state, model parameters, stage timing or pathway selection.

namespace stellaris {

enum class QualityPreset { Performance = 0, Balanced, High, Capture };
inline constexpr int kQualityPresetCount = 4;

struct RenderQuality {
    const char* name;
    float volumeScale;       // volume render-target size relative to the window
    int   volumeSteps;       // ray-march iterations (fixed step length = cloud diameter / steps)
    float particleFraction;  // share of particles drawn; brightness is compensated
};

inline RenderQuality renderQuality(QualityPreset p) {
    switch (p) {
    case QualityPreset::Performance: return {"Performance", 1.0f / 3.0f, 64, 0.5f};
    case QualityPreset::Balanced:    return {"Balanced", 0.5f, 96, 1.0f};
    case QualityPreset::High:        return {"High", 0.75f, 128, 1.0f};
    case QualityPreset::Capture:     return {"Capture", 1.0f, 160, 1.0f};
    }
    return {"Balanced", 0.5f, 96, 1.0f};
}

} // namespace stellaris
