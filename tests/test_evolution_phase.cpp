#include "test_harness.h"

#include "sim/EvolutionPhase.h"
#include "sim/Timeline.h"

#include <cstring>

using namespace stellaris;

TEST(evolution_phases_are_contiguous_and_ordered) {
    for (Pathway p : {Pathway::WhiteDwarf, Pathway::NeutronStar, Pathway::BlackHole}) {
        int prev = -1;
        for (double t = 0.0; t <= timelineDuration(); t += 0.05) {
            const EvolutionPhase ph = evolutionPhase(t, p, 1.0);
            CHECK(ph.index >= prev && ph.index < ph.count && ph.name && ph.name[0]);
            CHECK(ph.index - prev <= 1 || prev < 0);  // never skips a phase
            prev = ph.index;
        }
        CHECK(prev == evolutionPhase(0.0, p, 1.0).count - 1);  // ends in the last phase
    }
}

TEST(evolution_phases_respect_the_pathways) {
    auto seen = [](Pathway p, const char* word) {
        for (double t = 100.0; t <= timelineDuration(); t += 0.05)
            if (std::strstr(evolutionPhase(t, p, 1.0).name, word)) return true;
        return false;
    };
    CHECK(seen(Pathway::WhiteDwarf, "Planetary nebula") && seen(Pathway::WhiteDwarf, "White dwarf"));
    CHECK(!seen(Pathway::WhiteDwarf, "supernova") && !seen(Pathway::WhiteDwarf, "Neutron"));
    CHECK(seen(Pathway::NeutronStar, "supernova") && seen(Pathway::NeutronStar, "Neutron star"));
    CHECK(!seen(Pathway::NeutronStar, "Planetary") && !seen(Pathway::NeutronStar, "Black hole"));
    CHECK(seen(Pathway::BlackHole, "Black hole") && !seen(Pathway::BlackHole, "Neutron star"));
    // Helium flash only below ~2 M_sun.
    const EvolutionPhase lo = evolutionPhase(108.6, Pathway::WhiteDwarf, 1.0);
    const EvolutionPhase hi = evolutionPhase(108.6, Pathway::WhiteDwarf, 4.0);
    CHECK(std::strstr(lo.name, "Helium") && !std::strstr(hi.detail, "flash in"));
}
