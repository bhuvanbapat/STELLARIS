#pragma once
// Catalog of authentic, attributed astronomical observations shown in the
// Observation view. The JSON sidecars in assets/observations/ are the single
// source of truth (provenance per CLAUDE.md §19.7); this module only reads them.
// Observations are references for comparison, never inputs to the model.

#include "sim/StellarModel.h"
#include "sim/Timeline.h"

#include <filesystem>
#include <map>
#include <string>
#include <vector>

namespace stellaris {

struct Observation {
    std::map<std::string, std::string> fields;  // every sidecar field, verbatim
    std::filesystem::path imagePath;

    const std::string& get(const std::string& key) const;
    bool matches(Stage stage, Pathway pathway) const;
};

// Parses a flat JSON object whose values are strings (the sidecar format).
// Returns false on malformed input. Supports \" \\ \/ \n \t and \uXXXX escapes.
bool parseFlatJson(const std::string& text, std::map<std::string, std::string>& out, std::string* error = nullptr);

// Loads every *.json sidecar in `dir` (sorted by file name). Invalid records are
// skipped and reported in `errors`.
std::vector<Observation> loadObservationCatalog(const std::filesystem::path& dir, std::vector<std::string>* errors = nullptr);

} // namespace stellaris
