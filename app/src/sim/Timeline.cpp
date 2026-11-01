#include "sim/Timeline.h"

#include <algorithm>

namespace stellaris {

namespace {
// Stage durations were chosen so each stage has time to be observed at 1x speed
// (total ~2.5 minutes). The final stage is longest because it contains a
// multi-phase evolutionary pathway.
constexpr std::array<StageSpan, kStageCount> kStages = {{
    {Stage::MolecularCloud, "Molecular Cloud",               "Cloud",         0.0,  18.0},
    {Stage::Collapse,       "Gravitational Collapse",        "Collapse",      18.0, 40.0},
    {Stage::Protostar,      "Protostar Formation",           "Protostar",     40.0, 58.0},
    {Stage::DiscAndJets,    "Accretion Disc & Bipolar Jets", "Disc & Jets",   58.0, 82.0},
    {Stage::MainSequence,   "Main-Sequence Star",            "Main Sequence", 82.0, 100.0},
    {Stage::StellarFate,    "Stellar Fate & Remnant",        "Fate",          100.0, 148.0},
}};
} // namespace

const std::array<StageSpan, kStageCount>& stageTable() { return kStages; }

const StageSpan& stageSpan(Stage s) { return kStages[static_cast<int>(s)]; }

double timelineDuration() { return kStages.back().end; }

Stage stageAt(double t) {
    t = std::clamp(t, 0.0, timelineDuration());
    for (const StageSpan& span : kStages) {
        if (t < span.end) return span.stage;
    }
    return Stage::StellarFate;
}

double stageProgress(double t, Stage s) {
    const StageSpan& span = stageSpan(s);
    return std::clamp((t - span.start) / (span.end - span.start), 0.0, 1.0);
}

// ---------------------------------------------------------------------------

void Playback::update(double realDeltaSeconds) {
    if (!playing_) return;
    const double dt = std::clamp(realDeltaSeconds, 0.0, kMaxRealStep);
    time_ = std::min(time_ + dt * speed(), timelineDuration());
    if (atEnd()) playing_ = false;
}

void Playback::play() {
    if (atEnd()) time_ = 0.0;
    playing_ = true;
}

void Playback::pause() { playing_ = false; }

void Playback::togglePlay() { playing_ ? pause() : play(); }

void Playback::reset() {
    time_       = 0.0;
    playing_    = false;
    speedIndex_ = kDefaultSpeedIndex;
}

void Playback::seek(double t) { time_ = std::clamp(t, 0.0, timelineDuration()); }

void Playback::step(double deltaTimelineSeconds) { seek(time_ + deltaTimelineSeconds); }

void Playback::selectStage(Stage s) { seek(stageSpan(s).start); }

void Playback::setSpeedIndex(int index) {
    speedIndex_ = std::clamp(index, 0, static_cast<int>(kSpeeds.size()) - 1);
}

void Playback::faster() { setSpeedIndex(speedIndex_ + 1); }
void Playback::slower() { setSpeedIndex(speedIndex_ - 1); }

bool Playback::atEnd() const { return time_ >= timelineDuration(); }

} // namespace stellaris
