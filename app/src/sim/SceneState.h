#pragma once
// SceneState = evaluateScene(time, config).
//
// This is a pure function: the entire visual state of the scene is derived from
// the timeline position and the user's model parameters, never accumulated frame
// by frame. That is what makes seeking, rewinding and stage selection exact.
//
// Unless noted, values are dimensionless presentation parameters (artistic or
// prescribed), not physical quantities. Scene distances are in arbitrary
// "scene units" with compressed, symbolic scale.

#include "sim/CloudField.h"
#include "sim/StellarModel.h"
#include "sim/Timeline.h"

#include <glm/vec3.hpp>

#include <array>

namespace stellaris {

struct ModelConfig {
    double massSolar = kDefaultMassSolar;  // initial stellar mass [M_sun], parameter
    double rotation  = 1.0;                // relative initial core rotation (dimensionless)
    uint32_t cloudSeed = CloudField::kDefaultSeed;  // procedural cloud morphology seed

    static constexpr double kMinRotation = 0.6;
    static constexpr double kMaxRotation = 1.4;
};

struct Annotation {
    glm::vec3   anchor{0.0f};  // world-space point the label refers to
    const char* text  = "";
    float       alpha = 0.0f;  // 0 = hidden
};
inline constexpr int kMaxAnnotations = 4;

struct SceneState {
    double  time          = 0.0;
    Stage   stage         = Stage::MolecularCloud;
    double  stageProgress = 0.0;
    Pathway pathway       = Pathway::WhiteDwarf;
    MainSequenceProperties ms{};

    // --- Molecular cloud & collapse ---------------------------------------
    float cloudDensity = 1.0f;  // multiplier on cloud volume density
    float collapse     = 0.0f;  // normalized collapse progress, 0..1 (prescribed)
    float infallClock  = 0.0f;  // timeline seconds since collapse began (drives particles)
    float coreGlow     = 0.0f;  // embedded central emission inside the cloud (artistic)
    float accretionBurst = 0.0f;  // episodic accretion outburst of the protostar, 0..1 (prescribed)
    float cavity       = 0.0f;  // bipolar outflow cavity opening, 0..1
    float dispersal    = 0.0f;  // envelope cleared by stellar feedback, 0..1

    // --- Central object: protostar -> main-sequence star -> giant ----------
    float     centralPresence  = 0.0f;
    float     centralRadius    = 0.0f;  // scene units (symbolic)
    float     centralTempK     = 0.0f;  // temperature used to choose the color
    glm::vec3 centralColor{1.0f};       // blackbody color of centralTempK
    float     centralIntensity = 0.0f;  // HDR emission multiplier (artistic)
    float     turbulence       = 0.0f;  // surface-pattern contrast/scale (artistic)
    float     protostarVeil    = 0.0f;  // obscuring envelope over a young protostar
    // Sun-like magnetic activity needs an outer convection zone, which stars hotter
    // than roughly 6200-7000 K lack. `convective` (0..1) is derived from the
    // calculated T_eff; `activity` gates spots, prominences, flare and structured
    // corona (parametric visual representations, not plasma simulation).
    float     convective       = 0.0f;
    float     activity         = 0.0f;
    float     flare            = 0.0f;  // deterministic flare pulse, 0..1 (presentation)
    float     atmosphere       = 0.0f;  // presence of the main-sequence chromosphere/corona layers
    // Stellar wind particle species (illustrative strength/speed, see SCIENCE_NOTES):
    // on the main sequence it scales with log10(L); giants lose mass in a slow wind.
    float     wind             = 0.0f;
    float     windSpeed        = 0.0f;  // scene units per timeline second
    float     windDusty        = 0.0f;  // 0 = hot ionized wind color, 1 = cool dusty giant wind

    // --- Disc and jets ----------------------------------------------------
    float rotation        = 1.0f;  // copy of the config parameter
    float discScale       = 7.0f;  // centrifugal-radius scale (scene units), grows with rotation^2
    float discPresence    = 0.0f;
    float discOuterRadius = 0.0f;  // current visible outer edge (grows as high-j gas lands)
    float discInnerRadius = 0.0f;
    float jetPresence     = 0.0f;
    float jetLength       = 0.0f;

    // --- Final stage -------------------------------------------------------
    float     fateProgress     = 0.0f;  // progress through stage 6, 0..1
    float     giant            = 0.0f;  // giant / supergiant expansion, 0..1
    float     ejectaPresence   = 0.0f;
    float     ejectaClock      = 0.0f;  // timeline seconds since ejection began
    int       ejectaKind       = 0;     // 0 = planetary nebula shells, 1 = supernova ejecta
    float     ejectaGlow       = 0.0f;  // ionization of the planetary nebula by the hot core
    float     ejectaStrength   = 1.0f;  // supernova energy scale (smaller for the BH pathway)
    float     supernovaFlash   = 0.0f;  // transient brightness, 0..1
    float     remnantPresence  = 0.0f;
    float     remnantRadius    = 0.0f;  // scene units, greatly enlarged for visibility
    glm::vec3 remnantColor{1.0f};
    float     remnantTempK     = 0.0f;  // remnant surface temperature (prescribed, K)
    float     remnantIntensity = 0.0f;
    float     pulsarBeams      = 0.0f;
    float     blackHole        = 0.0f;  // presence of the black-hole lensing effect
    float     bhShadowRadius   = 0.0f;  // scene units (symbolic)
    float     bhEinsteinRadius = 0.0f;  // scene units (symbolic)

    // --- Presentation -----------------------------------------------------
    float exposure = 1.0f;
    std::array<Annotation, kMaxAnnotations> annotations{};
    int annotationCount = 0;
};

// Visual (symbolic) radius of a main-sequence star of the given radius [R_sun].
float visualStarRadius(double radiusSolar);

SceneState evaluateScene(double time, const ModelConfig& config);

} // namespace stellaris
