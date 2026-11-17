#include "test_harness.h"

#include "sim/ObservationCatalog.h"

#include <fstream>

using namespace stellaris;

namespace {
std::filesystem::path assetDir() { return std::filesystem::path(STELLARIS_SOURCE_DIR) / "assets" / "observations"; }

// Curated (hand-checked) images only; the acquired library adds "lib_" entries.
std::vector<std::string> idsFor(const std::vector<Observation>& catalog, Stage s, Pathway p) {
    std::vector<std::string> ids;
    for (const Observation& o : catalog)
        if (o.matches(s, p) && o.get("id").rfind("lib_", 0) != 0) ids.push_back(o.get("id"));
    return ids;
}
} // namespace

TEST(flat_json_parser_handles_strings_and_escapes) {
    // Inputs and expectations live in variables: MSVC mis-stringizes string literals
    // with escapes when they appear inside the CHECK macro.
    const char bs = '\\';
    const std::string input = std::string("{\"a\": \"x\", \"b\": \"q") + bs + "\"uote " + bs + bs + " back" + bs +
                              "nline\", \"c\": \"" + bs + "u00b5m\"}";
    const std::string expectedB = std::string("q") + '"' + "uote " + bs + " back" + '\n' + "line";
    const std::string expectedC = "\xC2\xB5m";  // UTF-8 for U+00B5
    std::map<std::string, std::string> m;
    const bool parsed = parseFlatJson(input, m);
    CHECK(parsed);
    CHECK(m["a"] == "x");
    CHECK(m["b"] == expectedB);
    CHECK(m["c"] == expectedC);
    std::map<std::string, std::string> empty;
    const bool parsedEmpty = parseFlatJson("{ }", empty);
    CHECK(parsedEmpty && empty.empty());
    const std::string nonString = "{\"a\": 1}";
    const std::string unterminated = "{\"a\": \"x\"";
    const std::string badEscape = std::string("{\"a\": \"x") + bs + "q\"}";
    std::map<std::string, std::string> bad;
    const bool r1 = parseFlatJson(nonString, bad);
    const bool r2 = parseFlatJson(unterminated, bad);
    const bool r3 = parseFlatJson(badEscape, bad);
    CHECK(!r1);  // non-string value
    CHECK(!r2);  // unterminated object
    CHECK(!r3);  // unknown escape
}

TEST(observation_catalog_loads_every_attributed_asset) {
    std::vector<std::string> errors;
    const auto catalog = loadObservationCatalog(assetDir(), &errors);
    CHECK(errors.empty());
    CHECK(catalog.size() >= 7);  // 7 curated images plus the acquired reference library
    for (const Observation& o : catalog) {
        CHECK(!o.get("credit").empty() && !o.get("license").empty() && !o.get("source_page").empty());
        CHECK(!o.get("band").empty() && !o.get("image_kind").empty() && !o.get("sha256").empty());
        CHECK(std::filesystem::exists(o.imagePath));
    }
}

TEST(observations_are_linked_to_the_right_stage_and_pathway) {
    const auto catalog = loadObservationCatalog(assetDir());
    using V = std::vector<std::string>;
    // Cloud and collapse: the same object in two genuine bands (visible + near-infrared).
    CHECK((idsFor(catalog, Stage::MolecularCloud, Pathway::WhiteDwarf) == V{"heic1501a", "weic2216a"}));
    CHECK((idsFor(catalog, Stage::Collapse, Pathway::BlackHole) == V{"heic1501a", "weic2216a"}));
    CHECK((idsFor(catalog, Stage::DiscAndJets, Pathway::NeutronStar) == V{"weic2322a"}));
    CHECK((idsFor(catalog, Stage::MainSequence, Pathway::WhiteDwarf) == V{"svs4277"}));
    // Final stage: one per pathway, never another pathway's object.
    CHECK((idsFor(catalog, Stage::StellarFate, Pathway::WhiteDwarf) == V{"weic2207a"}));
    CHECK((idsFor(catalog, Stage::StellarFate, Pathway::NeutronStar) == V{"heic0515a"}));
    CHECK((idsFor(catalog, Stage::StellarFate, Pathway::BlackHole) == V{"eso1907a"}));
    // No library image may be linked to the wrong ending (e.g. a Type Ia remnant).
    for (const Observation& o : catalog) {
        if (o.get("pathway") == "white-dwarf") CHECK(!o.matches(Stage::StellarFate, Pathway::NeutronStar));
        if (o.get("pathway") == "neutron-star") CHECK(!o.matches(Stage::StellarFate, Pathway::WhiteDwarf));
    }
}

TEST(observation_catalog_rejects_records_without_provenance) {
    const auto dir = std::filesystem::temp_directory_path() / "stellaris_obs_test";
    std::filesystem::create_directories(dir);
    { std::ofstream(dir / "img.jpg") << "x"; }
    { std::ofstream(dir / "a.json") << R"({"id": "a", "local_file": "img.jpg", "credit": "", "license": "CC BY 4.0"})"; }
    { std::ofstream(dir / "b.json") << R"({"id": "b", "local_file": "img.jpg", "credit": "Someone", "license": "CC BY 4.0"})"; }
    std::vector<std::string> errors;
    const auto catalog = loadObservationCatalog(dir, &errors);
    CHECK(catalog.size() == 1 && catalog[0].get("id") == "b");
    CHECK(errors.size() == 1);
    std::filesystem::remove_all(dir);
}
