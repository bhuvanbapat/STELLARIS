#include "test_harness.h"

#include "sim/Timeline.h"

using namespace stellaris;

TEST(stage_table_is_contiguous_and_ordered) {
    const auto& table = stageTable();
    CHECK(table.front().start == 0.0);
    for (int i = 0; i < kStageCount; ++i) {
        CHECK(static_cast<int>(table[i].stage) == i);
        CHECK(table[i].end > table[i].start);
        if (i > 0) CHECK(table[i].start == table[i - 1].end);
    }
    CHECK(timelineDuration() == table.back().end);
}

TEST(stage_lookup_at_boundaries) {
    for (const StageSpan& s : stageTable()) {
        CHECK(stageAt(s.start) == s.stage);
        CHECK(stageAt(s.start + 0.001) == s.stage);
    }
    CHECK(stageAt(-10.0) == Stage::MolecularCloud);
    CHECK(stageAt(timelineDuration()) == Stage::StellarFate);
    CHECK(stageAt(1e9) == Stage::StellarFate);
    CHECK_NEAR(stageProgress(29.0, Stage::Collapse), 0.5, 1e-9);
    CHECK(stageProgress(0.0, Stage::Collapse) == 0.0);
    CHECK(stageProgress(200.0, Stage::Collapse) == 1.0);
}

TEST(playback_initial_state_and_reset) {
    Playback p;
    CHECK(p.time() == 0.0);
    CHECK(!p.playing());
    CHECK(p.speed() == 1.0);
    p.seek(50.0);
    p.setSpeedIndex(4);
    p.play();
    p.reset();
    CHECK(p.time() == 0.0);
    CHECK(!p.playing());
    CHECK(p.speed() == 1.0);
}

TEST(playback_only_advances_while_playing) {
    Playback p;
    p.update(0.05);
    CHECK(p.time() == 0.0);
    p.play();
    p.update(0.05);
    CHECK_NEAR(p.time(), 0.05, 1e-12);
    p.pause();
    p.update(0.05);
    CHECK_NEAR(p.time(), 0.05, 1e-12);
}

TEST(playback_speed_scales_rate_not_position) {
    Playback p;
    p.seek(10.0);
    p.setSpeedIndex(4);  // 4x
    p.play();
    p.update(0.05);
    CHECK_NEAR(p.time(), 10.2, 1e-9);
    p.setSpeedIndex(0);  // 0.25x: changing speed keeps the position
    CHECK_NEAR(p.time(), 10.2, 1e-9);
    p.update(0.04);
    CHECK_NEAR(p.time(), 10.21, 1e-9);
    p.setSpeedIndex(99);
    CHECK(p.speedIndex() == 4);
    p.setSpeedIndex(-3);
    CHECK(p.speedIndex() == 0);
}

TEST(playback_large_wall_clock_steps_are_clamped) {
    Playback p;
    p.play();
    p.update(5.0);  // e.g. window dragged for 5 s
    CHECK_NEAR(p.time(), Playback::kMaxRealStep, 1e-12);
}

TEST(playback_stops_at_end_and_play_restarts) {
    Playback p;
    p.seek(timelineDuration() - 0.01);
    p.play();
    p.update(0.05);
    CHECK(p.time() == timelineDuration());
    CHECK(!p.playing());
    CHECK(p.atEnd());
    p.play();
    CHECK(p.time() == 0.0);
    CHECK(p.playing());
}

TEST(seek_step_and_stage_selection_clamp) {
    Playback p;
    p.seek(-5.0);
    CHECK(p.time() == 0.0);
    p.seek(1e6);
    CHECK(p.time() == timelineDuration());
    p.seek(20.0);
    p.step(-30.0);
    CHECK(p.time() == 0.0);
    for (const StageSpan& s : stageTable()) {
        p.selectStage(s.stage);
        CHECK(p.time() == s.start);
        CHECK(stageAt(p.time()) == s.stage);
    }
}
