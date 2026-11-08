#include "sim/StageContent.h"

namespace stellaris {

StageContent stageContent(Stage stage, Pathway pathway) {
    switch (stage) {
    case Stage::MolecularCloud:
        return {
            "Cold gas and dust in a molecular cloud",
            "Stars form in the coldest, densest parts of molecular clouds: vast regions of mostly "
            "molecular hydrogen with a little dust, at roughly 10–20 K. Turbulence, magnetic "
            "fields and gas pressure support most of the cloud, and only a small fraction of its "
            "mass ever becomes stars.",
            "An illustrative, procedurally generated cloud, not a real object. Colors are artistic; "
            "the shading imitates dust scattering blue light more strongly than red.",
        };
    case Stage::Collapse:
        return {
            "Gravity overcomes support in a dense core",
            "A dense core that becomes gravitationally unstable starts to contract. The densest "
            "inner gas has the shortest free-fall time, so it collapses first (“inside-out” "
            "collapse) while the rest of the cloud remains. Released gravitational energy heats the "
            "gas and is largely radiated away.",
            "Each particle follows a prescribed pressureless free-fall trajectory timed by its "
            "starting radius. Pressure, magnetic fields and turbulence are not simulated.",
        };
    case Stage::Protostar:
        return {
            "Accretion builds a central protostar",
            "Infalling gas piles up at the center into a hot, dense protostar still buried in its "
            "dusty envelope. It shines from the heat of contraction and accretion; sustained "
            "hydrogen fusion has not yet begun, so it is not a main-sequence star.",
            "Size and color are illustrative. It is drawn larger than the star it becomes because "
            "protostars contract as they evolve.",
        };
    case Stage::DiscAndJets:
        return {
            "Rotation forms a disc; jets clear the poles",
            "Because the core was rotating, infalling gas cannot fall straight in. It settles into a "
            "rotating accretion disc and spirals inward onto the star. Bipolar jets launched near "
            "the star, thought to be driven magnetically, carry away angular momentum along the "
            "axis, but most of the material is still accreted.",
            "Gas lands on the disc at its centrifugal radius (r ∝ j²) and then orbits "
            "with Keplerian speed (ω ∝ r^-1.5) and a prescribed inward "
            "drift. Jets are a procedural particle stream; magnetohydrodynamics is not modeled.",
        };
    case Stage::MainSequence:
        return {
            "Sustained hydrogen fusion in the core",
            "Hydrogen fusion into helium in the core now supplies the star's energy and halts its "
            "contraction. The star settles into equilibrium on the main sequence, where it spends "
            "most of its life. More massive stars are hotter, far more luminous and much "
            "shorter-lived.",
            "Luminosity, radius, temperature and lifetime come from simple scaling relations; the "
            "color is the computed blackbody color. Surface pattern, spots, prominences and the flare "
            "are illustrative (shown only for stars with an outer convection zone), not simulated; "
            "the star's size on screen is strongly compressed.",
        };
    case Stage::StellarFate:
        switch (pathway) {
        case Pathway::WhiteDwarf:
            return {
                "Giant phase, envelope loss, white dwarf",
                "With core hydrogen exhausted, the star swells into a red giant. Winds and "
                "pulsations later shed its outer layers, which glow as a planetary nebula when lit "
                "by the exposed hot core. The core remains as a white dwarf: an Earth-sized remnant "
                "that slowly cools over billions of years. No supernova occurs.",
                "The pathway follows illustrative mass categories (boundary near 8 M☉, really "
                "uncertain). Nebula shape and colors are artistic; the white dwarf is enormously "
                "enlarged.",
            };
        case Pathway::NeutronStar:
            return {
                "Core collapse, supernova, neutron star",
                "A massive star fuses heavier and heavier elements until an iron core forms that can "
                "no longer support itself. It collapses in about a second, and the rebound can drive "
                "a core-collapse supernova. The core may survive as a neutron star about 20 km "
                "across; some are observed as pulsars.",
                "Outcome boundaries (~8 and ~25 M☉) are illustrative: real outcomes also depend "
                "on composition, rotation, mass loss and companions. The neutron star is enlarged and "
                "the beam rotation is slowed.",
            };
        case Pathway::BlackHole:
            return {
                "Core collapse to a black hole",
                "For some of the most massive stars the collapsing core continues past the "
                "neutron-star stage and forms a black hole. Whether a bright supernova accompanies "
                "this is uncertain; some may collapse with only a faint explosion. The black hole "
                "emits no light of its own.",
                "Shown through a dark shadow and simplified point-mass lensing of background light. "
                "Not every massive star becomes a black hole; the ~25 M☉ boundary is illustrative.",
            };
        }
        break;
    }
    return {"", "", ""};
}

} // namespace stellaris
