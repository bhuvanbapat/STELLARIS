#include "sim/EvolutionPhase.h"

#include "sim/Timeline.h"

#include <vector>

namespace stellaris {

namespace {

struct Entry {
    double end;  // timeline seconds (or fate progress u, see below) where the phase ends
    const char* name;
    const char* detail;
    const char* duration;
};

// Stages 1-5: shared by every pathway (timeline seconds; aligned with SceneState).
const Entry kEarly[] = {
    {10.0, "Cold molecular cloud", "Gas and dust at 10-20 K, supported by turbulence and magnetic fields", "~10 Myr"},
    {18.0, "Fragmentation into dense cores", "Turbulence and gravity carve filaments; dense cores become unstable", "~1 Myr"},
    {30.0, "Isothermal free-fall collapse", "The core collapses inside-out; released heat still radiates away", "~0.1 Myr"},
    {36.0, "First hydrostatic core", "The centre turns opaque, traps its heat and briefly stops collapsing", "~1,000 yr"},
    {40.0, "Second collapse", "At ~2,000 K molecular hydrogen dissociates and the core collapses again", "~years"},
    {50.0, "Class 0 protostar", "A tiny protostar deep inside an envelope more massive than itself", "~0.05 Myr"},
    {58.0, "Class I protostar", "The envelope thins; most mass now reaches the star through the disc", "~0.5 Myr"},
    {70.0, "Disc and bipolar outflow", "Magnetized disc winds and jets carry angular momentum away", "~0.5 Myr"},
    {76.0, "T Tauri star (Class II)", "Envelope cleared; the star shines through, still accreting from its disc", "~1-10 Myr"},
    {82.0, "Pre-main-sequence contraction", "Slow contraction heats the core toward hydrogen ignition", "~10-30 Myr"},
    {86.0, "Zero-age main sequence", "Core hydrogen fusion starts and balances gravity", "-"},
    {100.0, "Core hydrogen burning", "Hydrogen fuses to helium in the core; the star is stable", "~10 Myr - 10 Gyr"},
};

// Stage 6 phases, by fate progress u in [0, 1] (aligned with evaluateFate()).
const Entry kWhiteDwarf[] = {
    {0.06, "Subgiant", "Core hydrogen exhausted; hydrogen burns in a shell and the envelope swells", "~1 Gyr"},
    {0.16, "Red-giant branch", "The envelope expands to ~100 R_sun and cools; the core contracts", "~0.5 Gyr"},
    {0.20, "Helium ignition and horizontal branch", "Core helium burning begins (a helium flash in stars below ~2 M_sun)", "~0.1 Gyr"},
    {0.30, "Asymptotic giant branch", "Helium and hydrogen burn in shells; thermal pulses and strong mass loss", "~1 Myr"},
    {0.50, "Post-AGB envelope ejection", "The last envelope is lifted off and the hot core is exposed", "~10,000 yr"},
    {0.68, "Planetary nebula", "Ultraviolet light of the >100,000 K core ionizes the expanding shells", "~10,000-50,000 yr"},
    {1.00, "White dwarf cooling", "No fusion; an Earth-sized carbon-oxygen core cools for billions of years", "> 10 Gyr"},
};
const Entry kMassive[] = {
    {0.06, "Crossing to the supergiant phase", "The core contracts after hydrogen runs out; the envelope expands fast", "~10,000 yr"},
    {0.285, "Red supergiant: advanced burning", "Helium, then carbon, neon, oxygen and silicon burn in nested shells", "~1 Myr (silicon: ~1 day)"},
    {0.30, "Iron-core collapse", "Fusion cannot proceed past iron; the core collapses at ~1/4 light speed", "< 1 s"},
    {0.42, "Core-collapse supernova", "The rebounding shock, powered by neutrinos, blows the star apart", "weeks to months (light)"},
    {0.62, "Expanding supernova remnant", "Ejecta and the shock sweep up the surrounding gas", "~10,000-100,000 yr"},
    {1.00, "Compact remnant", "", ""},
};

} // namespace

EvolutionPhase evolutionPhase(double t, Pathway pathway, double massSolar) {
    const bool wd = pathway == Pathway::WhiteDwarf;
    const auto& late = wd ? kWhiteDwarf : kMassive;
    const int nEarly = static_cast<int>(std::size(kEarly));
    const int nLate = wd ? static_cast<int>(std::size(kWhiteDwarf)) : static_cast<int>(std::size(kMassive));
    const int count = nEarly + nLate;
    const double fateStart = stageSpan(Stage::StellarFate).start;
    if (t < fateStart) {
        for (int i = 0; i < nEarly; ++i)
            if (t < kEarly[i].end || i == nEarly - 1)
                return {kEarly[i].name, kEarly[i].detail, kEarly[i].duration, i, count};
    }
    const double u = stageProgress(t, Stage::StellarFate);
    for (int i = 0; i < nLate; ++i) {
        if (u < late[i].end || i == nLate - 1) {
            EvolutionPhase p{late[i].name, late[i].detail, late[i].duration, nEarly + i, count};
            if (wd && i == 2 && massSolar >= 2.0)
                p.detail = "Core helium burning starts quietly (no helium flash above ~2 M_sun)";
            if (!wd && i == nLate - 1) {
                const bool bh = pathway == Pathway::BlackHole;
                p.name = bh ? "Black hole" : "Neutron star / pulsar";
                p.detail = bh ? "The core collapses beyond neutron degeneracy into a black hole"
                              : "A ~20 km neutron star, spinning and strongly magnetized";
                p.duration = "indefinite (pulsar emission: ~10 Myr)";
            }
            return p;
        }
    }
    return {late[nLate - 1].name, late[nLate - 1].detail, late[nLate - 1].duration, count - 1, count};
}

} // namespace stellaris
