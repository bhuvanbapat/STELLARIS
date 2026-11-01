#pragma once
// Concise on-screen scientific text for each stage. Sources and the full list of
// assumptions are in docs/SCIENCE_NOTES.md. Strings are UTF-8.

#include "sim/StellarModel.h"
#include "sim/Timeline.h"

namespace stellaris {

struct StageContent {
    const char* process;        // the principal physical process (one line)
    const char* concept;        // the scientific concept (2-3 sentences)
    const char* approximation;  // what the visualization simplifies or invents
};

StageContent stageContent(Stage stage, Pathway pathway);

} // namespace stellaris
