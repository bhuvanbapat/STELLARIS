#include "test_harness.h"

#include "render/ParticleSystem.h"
#include "sim/CameraPath.h"
#include "sim/SceneState.h"
#include "sim/StageContent.h"

#include <glm/geometric.hpp>

#include <cstring>

using namespace stellaris;

namespace {

// Continuous scene parameters that must never jump between nearby times.
std::vector<float> continuousFields(const SceneState& s) {
    return {s.cloudDensity, s.collapse, s.coreGlow, s.cavity, s.dispersal, s.centralPresence,
            s.protostarVeil, s.discPresence, s.jetPresence, s.giant, s.ejectaPresence, s.ejectaGlow,
            s.supernovaFlash, s.remnantPresence, s.pulsarBeams, s.blackHole, s.activity, s.flare, s.atmosphere, s.accretionBurst, s.streamers, s.giantAtmosphere, s.circumstellarRing};
}

bool sameState(const SceneState& a, const SceneState& b) {
    const auto fa = continuousFields(a), fb = continuousFields(b);
    return fa == fb && a.centralRadius == b.centralRadius && a.centralTempK == b.centralTempK &&
           a.exposure == b.exposure && a.stage == b.stage && a.annotationCount == b.annotationCount;
}

} // namespace

TEST(scene_evaluation_is_deterministic) {
    const ModelConfig cfg{12.0, 1.2};
    for (double t = 0.0; t <= timelineDuration(); t += 3.7) {
        const SceneState a = evaluateScene(t, cfg);
        evaluateScene(t + 50.0, cfg);  // unrelated evaluation in between: no hidden state
        const SceneState b = evaluateScene(t, cfg);
        CHECK(sameState(a, b));
    }
}

TEST(seeking_backward_matches_forward) {
    const ModelConfig cfg{};
    // Visiting a time after "playing through" vs jumping straight to it.
    std::vector<SceneState> forward;
    for (double t = 0.0; t <= 148.0; t += 1.0) forward.push_back(evaluateScene(t, cfg));
    for (int i = static_cast<int>(forward.size()) - 1; i >= 0; --i)
        CHECK(sameState(forward[i], evaluateScene(static_cast<double>(i), cfg)));
}

TEST(scene_parameters_are_continuous_in_time) {
    for (double mass : {1.0, 15.0, 40.0}) {
        const ModelConfig cfg{mass, 1.0};
        SceneState prev = evaluateScene(0.0, cfg);
        float maxJump = 0.0f, maxRadiusJump = 0.0f, maxTempJump = 0.0f;
        for (double t = 0.005; t <= timelineDuration(); t += 0.005) {
            const SceneState cur = evaluateScene(t, cfg);
            const auto a = continuousFields(prev), b = continuousFields(cur);
            for (size_t i = 0; i < a.size(); ++i) maxJump = std::max(maxJump, std::fabs(a[i] - b[i]));
            maxRadiusJump = std::max(maxRadiusJump, std::fabs(cur.centralRadius - prev.centralRadius));
            maxTempJump = std::max(maxTempJump, std::fabs(cur.centralTempK - prev.centralTempK));
            prev = cur;
        }
        CHECK(maxJump < 0.03f);
        CHECK(maxRadiusJump < 0.1f);
        CHECK(maxTempJump < 300.0f);
    }
}

TEST(stage_boundaries_from_both_sides_are_close) {
    const ModelConfig cfg{};
    for (int i = 1; i < kStageCount; ++i) {
        const double b = stageTable()[i].start;
        const SceneState before = evaluateScene(b - 1e-4, cfg);
        const SceneState after = evaluateScene(b + 1e-4, cfg);
        CHECK(static_cast<int>(before.stage) == i - 1);
        CHECK(static_cast<int>(after.stage) == i);
        const auto fa = continuousFields(before), fb = continuousFields(after);
        for (size_t k = 0; k < fa.size(); ++k) CHECK(std::fabs(fa[k] - fb[k]) < 1e-3f);
    }
}

TEST(each_pathway_ends_in_its_own_remnant) {
    const double end = timelineDuration();
    const SceneState wd = evaluateScene(end, {1.0, 1.0});
    const SceneState ns = evaluateScene(end, {15.0, 1.0});
    const SceneState bh = evaluateScene(end, {40.0, 1.0});

    CHECK(wd.pathway == Pathway::WhiteDwarf && wd.remnantPresence > 0.99f && wd.blackHole == 0.0f);
    CHECK(wd.ejectaKind == 0 && wd.supernovaFlash == 0.0f);  // no supernova on this pathway
    CHECK(ns.pathway == Pathway::NeutronStar && ns.remnantPresence > 0.99f && ns.pulsarBeams > 0.99f);
    CHECK(ns.ejectaKind == 1 && ns.blackHole == 0.0f);
    CHECK(bh.pathway == Pathway::BlackHole && bh.blackHole > 0.99f && bh.remnantPresence == 0.0f);
    CHECK(bh.ejectaStrength < ns.ejectaStrength);
    for (const SceneState* s : {&wd, &ns, &bh}) CHECK(s->centralPresence < 0.01f);  // the star itself is gone
}

TEST(no_supernova_before_the_final_stage) {
    for (double t = 0.0; t < stageSpan(Stage::StellarFate).start; t += 0.5) {
        const SceneState s = evaluateScene(t, {40.0, 1.0});
        CHECK(s.supernovaFlash == 0.0f && s.ejectaPresence == 0.0f && s.blackHole == 0.0f);
    }
}

TEST(rotation_parameter_controls_disc_size) {
    const SceneState slow = evaluateScene(75.0, {1.0, ModelConfig::kMinRotation});
    const SceneState fast = evaluateScene(75.0, {1.0, ModelConfig::kMaxRotation});
    CHECK(fast.discOuterRadius > slow.discOuterRadius);
    const SceneState clamped = evaluateScene(75.0, {1.0, 99.0});
    CHECK(clamped.rotation == static_cast<float>(ModelConfig::kMaxRotation));
}

TEST(main_sequence_star_color_depends_on_mass) {
    const SceneState sunLike = evaluateScene(95.0, {1.0, 1.0});
    const SceneState massive = evaluateScene(95.0, {15.0, 1.0});
    CHECK(sunLike.stage == Stage::MainSequence);
    CHECK(massive.centralTempK > sunLike.centralTempK);
    CHECK(massive.centralColor.b > sunLike.centralColor.b);
    CHECK(massive.centralRadius > sunLike.centralRadius);
}

TEST(protostar_is_larger_than_its_main_sequence_star) {
    const ModelConfig cfg{};
    const SceneState proto = evaluateScene(55.0, cfg);
    const SceneState ms = evaluateScene(96.0, cfg);
    CHECK(proto.centralRadius > ms.centralRadius);
    CHECK(proto.centralTempK < ms.centralTempK);
}

TEST(camera_path_is_smooth_and_finite) {
    for (Pathway p : {Pathway::WhiteDwarf, Pathway::NeutronStar, Pathway::BlackHole}) {
        glm::vec3 prev = poseFromOrbit(authoredOrbit(0.0, p, 1.0f), {}).position;
        float maxStep = 0.0f;
        for (double t = 0.01; t <= timelineDuration(); t += 0.01) {
            const CameraPose pose = poseFromOrbit(authoredOrbit(t, p, 1.0f), {});
            CHECK(std::isfinite(pose.position.x) && std::isfinite(pose.position.y) && std::isfinite(pose.position.z));
            const float d = glm::length(pose.position);
            CHECK(d > 5.0f && d < 150.0f);
            maxStep = std::max(maxStep, glm::length(pose.position - prev));
            prev = pose.position;
        }
        CHECK(maxStep < 0.6f);  // no teleporting between viewpoints
    }
}

TEST(user_camera_offset_is_layered_and_removable) {
    const OrbitParams o = authoredOrbit(30.0, Pathway::WhiteDwarf, 1.0f);
    const CameraPose base = poseFromOrbit(o, {});
    const CameraPose moved = poseFromOrbit(o, {40.0f, 10.0f, 0.5f});
    CHECK(glm::length(base.position - moved.position) > 1.0f);
    CHECK_NEAR(glm::length(moved.position), glm::length(base.position) * 0.5, 1e-3);
    const CameraPose again = poseFromOrbit(o, {});
    CHECK(glm::length(base.position - again.position) < 1e-6f);
}

TEST(stage_content_is_complete) {
    for (Pathway p : {Pathway::WhiteDwarf, Pathway::NeutronStar, Pathway::BlackHole}) {
        for (int i = 0; i < kStageCount; ++i) {
            const StageContent c = stageContent(static_cast<Stage>(i), p);
            CHECK(std::strlen(c.process) > 10 && std::strlen(c.explanation) > 80 && std::strlen(c.approximation) > 40);
        }
    }
    // The protostar text must not call it a main-sequence star.
    CHECK(std::strstr(stageContent(Stage::Protostar, Pathway::WhiteDwarf).explanation, "not a main-sequence star"));
}

TEST(particle_generation_is_deterministic) {
    ParticleCounts small;
    small.ambient = 2000; small.core = 3000; small.jet = 500; small.ejecta = 700;
    const CloudField cloud = CloudField::build(1u, 32);
    const auto a = generateParticles(small, 7u, cloud);
    const auto b = generateParticles(small, 7u, cloud);
    const auto c = generateParticles(small, 8u, cloud);
    CHECK(static_cast<int>(a.size()) == small.total());
    CHECK(a.size() == b.size() && std::memcmp(a.data(), b.data(), a.size() * sizeof(ParticleVertex)) == 0);
    CHECK(std::memcmp(a.data(), c.data(), a.size() * sizeof(ParticleVertex)) != 0);
    int core = 0;
    for (const ParticleVertex& v : a) {
        if (v.pos[3] == 1.0f) {
            ++core;
            CHECK(std::sqrt(v.pos[0] * v.pos[0] + v.pos[1] * v.pos[1] + v.pos[2] * v.pos[2]) <= 13.0f + 1e-3f);
        }
        for (float s : v.seed) CHECK(s >= 0.0f && s < 1.0f);
    }
    CHECK(core == small.core);
}

TEST(surface_activity_only_for_convective_main_sequence_stars) {
    const SceneState sun = evaluateScene(94.0, {1.0, 1.0});
    CHECK(sun.convective > 0.99f && sun.activity > 0.9f && sun.atmosphere > 0.9f);
    // Hot, massive stars have no outer convection zone: no spots/prominences/flare.
    const SceneState hot = evaluateScene(94.0, {15.0, 1.0});
    CHECK(hot.convective == 0.0f && hot.activity == 0.0f && hot.flare == 0.0f);
    CHECK(hot.atmosphere > 0.9f);  // still has a (smooth) outer atmosphere layer
    // Not before the main sequence, not as a giant.
    CHECK(evaluateScene(60.0, {1.0, 1.0}).activity == 0.0f);
    CHECK(evaluateScene(60.0, {1.0, 1.0}).atmosphere == 0.0f);
    CHECK(evaluateScene(120.0, {1.0, 1.0}).activity == 0.0f);
}

TEST(flare_is_a_single_deterministic_event) {
    const ModelConfig cfg{1.0, 1.0};
    float peak = 0.0f;
    double peakT = 0.0;
    for (double t = 80.0; t < 110.0; t += 0.01) {
        const float f = evaluateScene(t, cfg).flare;
        if (f > peak) { peak = f; peakT = t; }
    }
    CHECK(peak > 0.9f);
    CHECK(peakT > 91.0 && peakT < 92.0);
    CHECK(evaluateScene(90.0, cfg).flare == 0.0f);
    CHECK(evaluateScene(105.0, cfg).flare < 0.01f);
    CHECK(evaluateScene(91.4, cfg).flare == evaluateScene(91.4, cfg).flare);
}

TEST(accretion_bursts_only_while_a_protostar_accretes) {
    const ModelConfig cfg{1.0, 1.0};
    float peak = 0.0f;
    for (double t = 0.0; t <= timelineDuration(); t += 0.05) {
        const float b = evaluateScene(t, cfg).accretionBurst;
        CHECK(b >= 0.0f && b <= 1.0f);
        if (t < 40.0 || t > 90.0) CHECK(b == 0.0f);
        peak = std::max(peak, b);
    }
    CHECK(peak > 0.5f);
    // A burst brightens the embedded source relative to the quiet state just before it.
    CHECK(evaluateScene(61.6, cfg).coreGlow > evaluateScene(60.9, cfg).coreGlow);
}
