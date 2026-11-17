#pragma once
// Named evolutionary phases inside the six narrative stages (e.g. Class 0 / Class I /
// T Tauri; subgiant / red-giant branch / horizontal branch / AGB / planetary nebula /
// white dwarf; red supergiant / core collapse / supernova / remnant). A pure function
// of timeline time, pathway and mass, aligned with the prescribed scene animation.
// The real durations differ by orders of magnitude (SCIENCE_NOTES §9.10).

#include "sim/StellarModel.h"

namespace stellaris {

struct EvolutionPhase {
    const char* name;        // e.g. "Red-giant branch"
    const char* detail;      // one line: what physically happens
    const char* duration;    // order of magnitude of the real duration
    int index = 0;           // 0-based position in the whole sequence for this pathway
    int count = 0;           // number of phases for this pathway
};

EvolutionPhase evolutionPhase(double timelineSeconds, Pathway pathway, double massSolar);

} // namespace stellaris
