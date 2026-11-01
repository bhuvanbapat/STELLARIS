#pragma once
// Narrative timeline: the six stages, their positions on the timeline, and the
// playback controller. Timeline seconds are presentation time, NOT physical time.

#include <array>

namespace stellaris {

enum class Stage : int {
    MolecularCloud = 0,
    Collapse,
    Protostar,
    DiscAndJets,
    MainSequence,
    StellarFate,
};
inline constexpr int kStageCount = 6;

struct StageSpan {
    Stage       stage;
    const char* name;       // full title, e.g. "Gravitational Collapse"
    const char* shortName;  // timeline label, e.g. "Collapse"
    double      start;      // timeline seconds (inclusive)
    double      end;        // timeline seconds (exclusive, except the last stage)
};

const std::array<StageSpan, kStageCount>& stageTable();
const StageSpan& stageSpan(Stage s);
double timelineDuration();

// Stage containing timeline position t (t is clamped to the timeline range).
Stage stageAt(double t);
// Progress of t through stage s, clamped to [0, 1].
double stageProgress(double t, Stage s);

// Playback controller. The only state is (time, playing, speed); the visual
// scene is always re-derived from time, so any operation here is safe in any order.
class Playback {
public:
    static constexpr std::array<double, 5> kSpeeds = {0.25, 0.5, 1.0, 2.0, 4.0};
    static constexpr int kDefaultSpeedIndex = 2;
    // Longest wall-clock step accepted per update (prevents jumps after stalls).
    static constexpr double kMaxRealStep = 0.1;

    // Advance by a wall-clock interval (seconds) if playing.
    void update(double realDeltaSeconds);

    void play();      // at the end of the timeline, play restarts from 0
    void pause();
    void togglePlay();
    void reset();     // time 0, paused, default speed
    void seek(double t);
    void step(double deltaTimelineSeconds);
    void selectStage(Stage s);
    void setSpeedIndex(int index);
    void faster();
    void slower();

    double time() const { return time_; }
    bool   playing() const { return playing_; }
    int    speedIndex() const { return speedIndex_; }
    double speed() const { return kSpeeds[speedIndex_]; }
    bool   atEnd() const;

private:
    double time_       = 0.0;
    bool   playing_    = false;
    int    speedIndex_ = kDefaultSpeedIndex;
};

} // namespace stellaris
