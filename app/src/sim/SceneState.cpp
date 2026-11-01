#include "sim/SceneState.h"

#include <algorithm>
#include <cmath>

namespace stellaris {

namespace {

float smooth(double e0, double e1, double x) {
    const double t = std::clamp((x - e0) / (e1 - e0), 0.0, 1.0);
    return static_cast<float>(t * t * (3.0 - 2.0 * t));
}

float mixf(float a, float b, float t) { return a + (b - a) * t; }

// Absolute timeline position of a fraction u of stage 6.
double fateTime(double u) {
    const StageSpan& s = stageSpan(Stage::StellarFate);
    return s.start + u * (s.end - s.start);
}

void addAnnotation(SceneState& s, glm::vec3 anchor, const char* text, float alpha) {
    if (alpha <= 0.01f || s.annotationCount >= kMaxAnnotations) return;
    s.annotations[s.annotationCount++] = Annotation{anchor, text, alpha};
}

// Fade in over [a, b], fade out over [c, d].
float window(double t, double a, double b, double c, double d) {
    return smooth(a, b, t) * (1.0f - smooth(c, d, t));
}

void evaluateFate(SceneState& s, double t, float msRadius) {
    const double u = stageProgress(t, Stage::StellarFate);
    s.fateProgress = static_cast<float>(t >= stageSpan(Stage::StellarFate).start ? u : 0.0);
    const bool inFate = t >= stageSpan(Stage::StellarFate).start;

    switch (s.pathway) {
    case Pathway::WhiteDwarf: {
        // Expansion into a giant, loss of the envelope as planetary-nebula shells,
        // exposure of the hot core, which remains as a white dwarf.
        s.giant = inFate ? smooth(0.02, 0.28, u) : 0.0f;
        const float shed = inFate ? smooth(0.30, 0.56, u) : 0.0f;
        s.centralRadius = mixf(s.centralRadius, 5.0f, s.giant) * (1.0f - 0.85f * shed);
        s.centralTempK  = mixf(s.centralTempK, 3300.0f, s.giant);
        s.turbulence    = mixf(s.turbulence, 1.0f, s.giant);
        s.centralPresence *= 1.0f - (inFate ? smooth(0.48, 0.58, u) : 0.0f);

        s.ejectaKind     = 0;
        s.ejectaPresence = inFate ? smooth(0.28, 0.36, u) : 0.0f;
        s.ejectaClock    = static_cast<float>(std::max(0.0, t - fateTime(0.28)));
        s.ejectaGlow     = inFate ? smooth(0.48, 0.68, u) : 0.0f;

        s.remnantPresence  = inFate ? smooth(0.50, 0.60, u) : 0.0f;
        s.remnantRadius    = 0.16f;
        s.remnantColor     = blackbodyColor(60000.0);
        s.remnantIntensity = 9.0f;
        break;
    }
    case Pathway::NeutronStar:
    case Pathway::BlackHole: {
        // Expansion into a supergiant, core collapse, supernova, compact remnant.
        const bool bh = s.pathway == Pathway::BlackHole;
        s.giant = inFate ? smooth(0.02, 0.27, u) : 0.0f;
        s.centralRadius = mixf(s.centralRadius, bh ? 8.5f : 7.5f, s.giant);
        s.centralTempK  = mixf(s.centralTempK, bh ? 3600.0f : 3500.0f, s.giant);
        s.turbulence    = mixf(s.turbulence, 1.0f, s.giant);
        s.centralPresence *= 1.0f - (inFate ? smooth(0.300, 0.318, u) : 0.0f);

        s.ejectaStrength = bh ? 0.35f : 1.0f;
        const float rise = inFate ? smooth(0.297, 0.306, u) : 0.0f;
        const double since = std::max(0.0, t - fateTime(0.306));
        s.supernovaFlash = rise * static_cast<float>(std::exp(-since / 3.5)) * s.ejectaStrength;

        s.ejectaKind     = 1;
        s.ejectaPresence = inFate ? smooth(0.298, 0.312, u) : 0.0f;
        s.ejectaClock    = static_cast<float>(std::max(0.0, t - fateTime(0.300)));
        s.ejectaGlow     = 1.0f;

        if (bh) {
            s.blackHole        = inFate ? smooth(0.31, 0.40, u) : 0.0f;
            s.bhShadowRadius   = 0.9f;
            s.bhEinsteinRadius = 2.4f;
        } else {
            s.remnantPresence  = inFate ? smooth(0.34, 0.42, u) : 0.0f;
            s.remnantRadius    = 0.12f;
            s.remnantColor     = blackbodyColor(40000.0);
            s.remnantIntensity = 12.0f;
            s.pulsarBeams      = inFate ? smooth(0.40, 0.50, u) : 0.0f;
        }
        break;
    }
    }
    (void)msRadius;
}

} // namespace

float visualStarRadius(double radiusSolar) {
    // Strongly compressed so a 1-40 M_sun range stays on screen.
    return static_cast<float>(std::pow(radiusSolar, 0.35));
}

SceneState evaluateScene(double time, const ModelConfig& config) {
    SceneState s;
    const double t = std::clamp(time, 0.0, timelineDuration());
    s.time          = t;
    s.stage         = stageAt(t);
    s.stageProgress = stageProgress(t, s.stage);
    s.pathway       = pathwayForMass(config.massSolar);
    s.ms            = mainSequenceProperties(config.massSolar);
    s.rotation      = static_cast<float>(
        std::clamp(config.rotation, ModelConfig::kMinRotation, ModelConfig::kMaxRotation));

    // Stage boundaries (timeline seconds) for readability below.
    const double tCollapse = stageSpan(Stage::Collapse).start;     // 18
    const double tMS       = stageSpan(Stage::MainSequence).start; // 82
    const double tFate     = stageSpan(Stage::StellarFate).start;  // 100

    // --- Cloud and collapse --------------------------------------------------
    s.cloudDensity = 1.0f - 0.55f * smooth(84, 100, t) - 0.27f * smooth(100, 114, t);
    s.collapse     = smooth(tCollapse, 54, t);
    s.infallClock  = static_cast<float>(std::max(0.0, t - tCollapse));
    s.coreGlow     = smooth(22, 44, t) * (1.0f - 0.8f * smooth(84, 102, t));
    s.cavity       = smooth(60, 80, t);
    s.dispersal    = smooth(84, 106, t);

    // --- Central object ------------------------------------------------------
    const float msRadius   = visualStarRadius(s.ms.radiusSolar);
    const float protostar  = smooth(36, 46, t);
    const float ignite     = smooth(tMS - 2.0, tMS + 6.0, t);
    // A protostar is larger than the main-sequence star it becomes and contracts
    // toward it (pre-main-sequence contraction). Factors here are illustrative.
    const float protoRadius = msRadius * (1.6f - 0.3f * smooth(46, tMS, t)) * mixf(0.35f, 1.0f, protostar);
    const float protoTemp   = mixf(2400.0f, 4000.0f, smooth(42, tMS, t)); // illustrative

    s.centralPresence  = protostar;
    s.centralRadius    = mixf(protoRadius, msRadius, ignite);
    s.centralTempK     = mixf(protoTemp, static_cast<float>(s.ms.teffK), ignite);
    s.turbulence       = mixf(1.0f, 0.35f, ignite);
    s.protostarVeil    = protostar * (1.0f - smooth(58, tMS, t)) * 0.75f;

    // --- Disc and jets -------------------------------------------------------
    // Centrifugal radius r_c = j^2 / (G M) scales as (initial angular velocity)^2.
    s.discScale       = std::clamp(7.0f * s.rotation * s.rotation, 2.5f, 11.0f);
    s.discPresence    = smooth(48, 60, t) * (1.0f - smooth(86, 98, t));
    s.discOuterRadius = s.discScale * mixf(0.25f, 1.0f, smooth(46, 72, t));
    s.discInnerRadius = msRadius * 1.25f;
    s.jetPresence     = smooth(58, 64, t) * (1.0f - smooth(84, 93, t));
    s.jetLength       = 22.0f * smooth(58, 70, t);

    // --- Final stage ------------------------------------------------------------
    evaluateFate(s, t, msRadius);

    s.centralColor = blackbodyColor(s.centralTempK);
    const float msIntensity = 7.0f * static_cast<float>(std::pow(s.ms.teffK / kSolarTeffK, 0.5));
    s.centralIntensity = mixf(2.6f, msIntensity, ignite) * mixf(1.0f, 0.55f, s.giant);

    // --- Exposure (authored "camera adaptation") ---------------------------------
    float exposure = 1.5f;
    exposure = mixf(exposure, 1.2f, smooth(30, 46, t));
    exposure = mixf(exposure, 0.95f, smooth(58, 70, t));
    exposure = mixf(exposure, 0.75f, smooth(tMS, tMS + 8, t));
    exposure = mixf(exposure, 1.1f, smooth(tFate + 2, tFate + 20, t));
    s.exposure = exposure / (1.0f + 5.0f * s.supernovaFlash);

    // --- Annotations ---------------------------------------------------------------
    addAnnotation(s, {0.0f, 2.0f, 0.0f}, "Dense core", window(t, 4, 8, 28, 34));
    addAnnotation(s, {0.0f, s.centralRadius, 0.0f}, "Protostar", window(t, 44, 48, 64, 68));
    addAnnotation(s, {s.discOuterRadius * 0.72f, 0.0f, 0.0f}, "Accretion disc", window(t, 62, 66, 84, 88));
    addAnnotation(s, {0.0f, s.jetLength * 0.55f, 0.0f}, "Bipolar jet", window(t, 64, 68, 84, 88));
    addAnnotation(s, {0.0f, s.centralRadius, 0.0f}, "Main-sequence star", window(t, 86, 89, 99, 101));
    if (s.pathway == Pathway::WhiteDwarf) {
        addAnnotation(s, {0.0f, s.centralRadius, 0.0f}, "Red giant", window(t, fateTime(0.12), fateTime(0.18), fateTime(0.36), fateTime(0.42)));
        addAnnotation(s, {0.0f, 4.0f + s.ejectaClock * 0.45f, 0.0f}, "Planetary nebula", window(t, fateTime(0.58), fateTime(0.64), 200, 201));
        addAnnotation(s, {0.0f, 0.2f, 0.0f}, "White dwarf (enlarged)", window(t, fateTime(0.62), fateTime(0.68), 200, 201));
    } else {
        addAnnotation(s, {0.0f, s.centralRadius, 0.0f}, "Red supergiant", window(t, fateTime(0.10), fateTime(0.16), fateTime(0.26), fateTime(0.29)));
        addAnnotation(s, {0.0f, 0.0f, 0.0f}, "Core collapse", window(t, fateTime(0.296), fateTime(0.300), fateTime(0.33), fateTime(0.36)));
        addAnnotation(s, {0.0f, 14.0f, 0.0f}, "Supernova remnant", window(t, fateTime(0.50), fateTime(0.56), 200, 201));
        if (s.pathway == Pathway::NeutronStar)
            addAnnotation(s, {0.0f, 0.2f, 0.0f}, "Neutron star (enlarged)", window(t, fateTime(0.62), fateTime(0.68), 200, 201));
        else
            addAnnotation(s, {0.0f, 1.0f, 0.0f}, "Black hole: event-horizon shadow", window(t, fateTime(0.62), fateTime(0.68), 200, 201));
    }
    return s;
}

} // namespace stellaris
