# STELLARIS — Project Specification

**Document type:** Authoritative Product and Functional Specification
**Project:** STELLARIS
**Domain:** Computer Graphics, Scientific Visualization, Stellar Formation and Evolution
**Target platform:** Windows desktop
**Primary user:** Student presenting a Computer Graphics academic project
**Status:** Requirements definition

---

# 1. Project Vision

STELLARIS is an interactive, cinematic 3D scientific visualization that takes the viewer through the formation and evolution of a star.

The experience begins inside an enormous molecular cloud, follows gravitational collapse and protostar formation, reveals an accretion disc and bipolar jets, transitions into a luminous main-sequence star, and concludes by showing the possible evolutionary outcomes determined by stellar mass.

The viewer must be able to observe, control, explore, and understand this progression.

The application should combine the visual ambition of a cinematic space experience with the clarity of a scientific educational visualization.

**The primary objective is to create a visually impressive, interactive Computer Graphics application with a coherent scientific narrative—not to build a complete astrophysical research simulator.**

The final result must be a real desktop application that builds, launches, and operates locally on the target Windows machine.

# 2. Core Objectives

The application must satisfy these objectives:

1. Present a coherent visual journey through stellar formation and evolution.
2. Demonstrate meaningful 3D graphics techniques.
3. Provide a cinematic visual experience with deliberate composition, lighting, depth, and motion.
4. Support interactive navigation through the stages.
5. Provide a timeline with predictable playback and seeking.
6. Represent scientific concepts accurately within a clearly defined simplified model.
7. Demonstrate different stellar evolutionary outcomes based on initial mass.
8. Run reliably on the target hardware.
9. Remain maintainable, understandable, and reproducibly buildable.
10. Support a live academic demonstration without requiring external services or a network connection during normal operation.

# 3. Target Environment

The primary development and execution environment is:

* Operating system: Windows 11.
* Graphics hardware: NVIDIA GeForce RTX 4050 Laptop GPU with 6 GB VRAM.
* Application type: Native desktop application.
* Development approach: Claude Code-assisted implementation.
* User programming experience: Beginner.

The graphics architecture and dependency versions must be selected after evaluating academic requirements and the available toolchain.

Do not assume a particular compiler, graphics API, rendering engine, or build system has already been configured.

Verify the actual environment before depending on it.

The application should use the available GPU effectively without requiring unnecessarily expensive rendering techniques.

# 4. Required Scientific Narrative

The application must present six major stages in the following order.

## Stage 1 — Molecular Cloud

**Concept:** The initial stellar nursery.

The viewer encounters a large, three-dimensional molecular cloud containing gas and dust.

Required behavior:

* Establish a strong sense of spatial scale and depth.
* Show an irregular cloud structure rather than a simple geometric sphere.
* Use layered density, particles, lighting, or volumetric techniques to communicate cloud structure.
* Provide subtle internal motion where appropriate.
* Introduce the scientific context through concise on-screen information.

The cloud must remain visually readable against the surrounding space.

## Stage 2 — Gravitational Collapse

**Concept:** A dense region of the cloud contracts under gravity.

Required behavior:

* Communicate the transition from a diffuse cloud to a denser central region.
* Show material moving inward in a coherent, controlled manner.
* Increase central concentration and luminosity as the stage progresses.
* Make the contraction understandable through changes in density, spatial distribution, and motion.
* Transition continuously from the initial cloud state.

Do not depict the entire cloud collapsing uniformly into a perfectly rigid sphere unless a specific visual approximation is clearly justified.

## Stage 3 — Protostar Formation

**Concept:** A central protostar develops from the collapsing material.

Required behavior:

* Establish a distinct central object.
* Show continued accretion or inward material movement where appropriate.
* Communicate the relationship between the central object and its surrounding material.
* Introduce controlled emission, lighting, and atmospheric effects.
* Preserve visual continuity with the collapse stage.

Do not describe the protostar as a fully developed main-sequence star.

## Stage 4 — Accretion Disc and Bipolar Jets

**Concept:** A young stellar object surrounded by a rotating disc, with bipolar outflows where represented.

Required behavior:

* Show a visually distinct disc around the central object.
* Communicate disc rotation.
* Establish a clear central axis.
* Represent bipolar jets extending away from the central region.
* Use particles, procedural geometry, shaders, or other appropriate techniques to communicate motion and structure.
* Maintain a readable camera composition that distinguishes the disc from the jets.

The visual presentation must not imply that all material is simply expelled from the system.

## Stage 5 — Main-Sequence Star

**Concept:** A stable star producing energy through hydrogen fusion in its core.

Required behavior:

* Transition to a distinct stellar appearance.
* Use appropriate lighting, shading, and surface effects to create a convincing luminous object.
* Communicate the main-sequence stage through concise scientific information.
* Distinguish visual surface effects from actual simulated stellar-interior physics.
* Preserve the user's ability to pause, seek, and inspect the stage.

Do not claim to simulate detailed nuclear reactions or stellar-interior structure unless those calculations are explicitly implemented and validated.

## Stage 6 — Mass-Dependent Stellar Evolution

**Concept:** A star's initial mass influences its evolutionary pathway and final remnant.

Required behavior:

* Explain that stars do not all end their lives in the same way.
* Present distinct evolutionary outcomes according to the approved scientific model.
* Include the low/intermediate-mass pathway leading toward a white dwarf.
* Include the high-mass pathway leading through a supernova to a neutron star or black hole, as appropriate to the model.
* Make the connection between initial mass and final outcome understandable.
* Avoid implying that a black hole is the inevitable endpoint for every massive star.
* Clearly label the final outcomes.

The exact mass thresholds and scientific qualifications must be established in `SCIENTIFIC_MODEL.md`.

Do not invent precise thresholds or present simplified thresholds as universal laws.

# 5. Interaction Requirements

The application must provide meaningful user control.

The following capabilities are required unless the final approved architecture documents a compelling technical limitation and the user explicitly accepts the change.

## 5.1 Cinematic Playback

The user must be able to:

* Start playback.
* Pause playback.
* Resume playback.
* Reset to the beginning.
* Move backward and forward through the timeline.
* Adjust playback speed through a small number of sensible options.

Controls must respond predictably.

## 5.2 Timeline

Provide a visible timeline containing the six major stages.

The timeline must:

* Show the current position.
* Indicate the current stage.
* Allow the user to seek to a chosen position.
* Support selecting a stage directly.
* Update the visualization when the user seeks.
* Remain synchronized with the animation.
* Avoid inconsistent states when the user moves rapidly between stages.

Seeking to a position must not require replaying the entire animation from the beginning.

## 5.3 Stage Information

Provide a concise information panel or equivalent presentation that communicates:

* The current stage.
* The principal physical process.
* The relevant scientific concept.
* Any important approximation in the visualization.

Information must remain readable without overwhelming the scene.

## 5.4 Limited Scientific Interaction

Provide a small number of meaningful scientific controls.

The final controls must be chosen after the scientific model and visual design are defined.

Prefer controls that visibly and meaningfully affect the visualization or explain an important physical relationship.

Do not add numerous sliders merely to make the interface appear sophisticated.

## 5.5 Input and Navigation

Provide a coherent way to interact with the 3D scene.

Camera controls, if included, must not interfere with timeline interaction.

The initial camera composition should be deliberately designed and should not require the user to manipulate the camera to see the main subject.

# 6. Animation Requirements

The animation system must support reliable playback and seeking.

Requirements:

1. Animation state must be deterministic for a given timeline position and configuration.
2. Seeking must reproduce the appropriate state.
3. Stage transitions must work in both forward and backward navigation.
4. Playback speed changes must not corrupt the animation state.
5. Reset must restore the defined initial state.
6. Stage selection must produce the correct scene state.
7. Animation behavior must not depend on the machine maintaining a particular frame rate.
8. Scene state must remain coherent after repeated seeking and playback.

Prefer a timeline-driven animation model over uncontrolled accumulation of frame-dependent transformations.

Where numerical simulation is used, distinguish simulation state from presentation state.

# 7. Scientific Model Requirements

The application must use a deliberately limited scientific model.

The model must be sufficient to support the intended visualization and educational narrative.

It must not pretend to reproduce the complete physics of real stellar formation.

The implementation must distinguish among:

1. **Calculated quantities:** values produced by implemented equations or numerical procedures.
2. **Parameterized quantities:** values selected as inputs or simplified model parameters.
3. **Artistic effects:** visual effects used to communicate a concept without claiming physical accuracy.
4. **Explanatory information:** scientific facts presented to the viewer.

The scientific model must document its assumptions and limitations.

Every displayed numerical quantity must have a clear meaning and appropriate units where applicable.

Do not fabricate measurements, experimental validation, or astrophysical accuracy claims.

Detailed model decisions belong in `SCIENTIFIC_MODEL.md`.

# 8. Computer Graphics Requirements

The final architecture must support the visual features required by the approved design.

The implementation should evaluate suitable techniques for:

* Three-dimensional rendering.
* Procedural geometry.
* GPU particle systems.
* Shader-based effects.
* Volumetric or density-field rendering where appropriate.
* Lighting and emissive materials.
* Camera movement and composition.
* Animation and interpolation.
* Post-processing effects where appropriate.
* Interactive interface rendering.
* Efficient resource management.

These are capabilities to evaluate, not a requirement to implement every technique regardless of value.

Select techniques based on their contribution to the final experience, technical feasibility, and academic requirements.

The project must document the important Computer Graphics techniques actually implemented.

Do not claim a graphics technique is implemented merely because a library theoretically supports it.

# 9. Visual Quality Requirements

The application must not resemble a generic introductory graphics assignment.

The visual experience should be coherent across all six stages.

Required quality characteristics:

* Strong composition.
* Convincing spatial depth.
* Deliberate lighting.
* Appropriate visual contrast.
* Carefully controlled color and emission.
* Smooth animation.
* Intentional camera behavior.
* Distinct stage identities.
* Coherent transitions.
* Readable interface elements.
* Stable visual output.
* No conspicuous rendering artifacts in the normal demonstration path.

Visual quality must be assessed using actual application output.

A collection of primitive shapes and generic particles is not sufficient as a final result if it fails to communicate the intended stellar phenomena.

The precise art direction, palette, typography, and rendering style will be specified in `VISUAL_DESIGN.md`.

# 10. Performance and Reliability

The application must be designed for the target laptop.

Requirements:

* Maintain responsive interaction.
* Avoid uncontrolled allocation of GPU or CPU resources.
* Release graphics resources correctly.
* Avoid unnecessary per-frame CPU work.
* Avoid unnecessary full-resolution effects.
* Use appropriate levels of rendering complexity.
* Handle window resizing and application shutdown correctly.
* Provide useful diagnostic information when failures occur.

Establish performance targets after the first functioning rendering prototype has been measured.

Do not invent performance measurements or claim that a frame rate has been achieved without testing.

A visual feature that causes severe instability must be optimized, redesigned, or explicitly reconsidered.

# 11. Offline Operation

The final application must be runnable locally after the development dependencies and required assets have been installed.

Normal application operation must not depend on:

* Remote APIs.
* External AI services.
* Cloud rendering.
* Network-hosted textures or models.
* Live internet access.

Any external dependency must be identified and documented.

Do not introduce unnecessary online services into a local academic visualization.

# 12. Build and Delivery Requirements

The project must include a reproducible build and execution workflow.

The exact tools and file structure will be established after the architecture is selected.

The completed project must include:

* Application source code.
* Required shaders and assets.
* Dependency and environment documentation.
* Build instructions.
* Run instructions.
* Scientific assumptions and limitations.
* Documentation of the implemented graphics techniques.
* Verification results and known limitations.

A successful deliverable must launch on the intended Windows environment.

Do not treat source generation alone as project completion.

# 13. Explicitly Out of Scope

Unless explicitly approved by the user, do not implement:

* A complete astrophysics engine.
* A research-grade hydrodynamics solver.
* Detailed nuclear reaction networks.
* A complete stellar-interior model.
* A full gravitational N-body simulation.
* An elaborate game framework.
* Missions, levels, enemies, or scoring.
* Multiplayer or networking.
* User accounts or databases.
* Cloud services.
* A large collection of scientific graphs.
* Unnecessary menus and settings.
* An elaborate editor for creating simulations.
* Unnecessary dependency frameworks.
* Features added merely to increase source-code volume.

The project should maximize the quality and coherence of the core experience rather than the number of features.

# 14. Academic Compliance

Academic requirements must be verified before finalizing the graphics architecture.

In particular, determine whether GLUT/freeGLUT is:

* Mandatory in the main STELLARIS application;
* Acceptable in a separate supporting demonstration; or
* Not required for the primary application.

Do not assume the answer.

If GLUT is mandatory, incorporate that requirement into the architecture before major implementation.

Document the graphics techniques and libraries actually used.

Do not misrepresent the application's capabilities or claim that a technique was implemented when it was not.

# 15. Acceptance Criteria

The project may be considered complete only when the following criteria have been verified.

## Functional

* [ ] The application builds successfully.
* [ ] The application launches successfully.
* [ ] All six stages are accessible.
* [ ] Playback, pause, resume, and reset work.
* [ ] Timeline seeking works.
* [ ] Direct stage selection works.
* [ ] Backward and forward seeking produce coherent states.
* [ ] Required scientific controls work.
* [ ] The interface remains responsive.

## Visual

* [ ] The molecular cloud communicates three-dimensional structure.
* [ ] Gravitational collapse is visually understandable.
* [ ] Protostar formation is distinct from the preceding stage.
* [ ] The accretion disc and bipolar jets are visually distinguishable.
* [ ] The main-sequence star is visually distinct.
* [ ] Stellar evolutionary outcomes are clearly differentiated.
* [ ] Transitions are coherent.
* [ ] The initial composition is visually deliberate.
* [ ] Rendering artifacts and major visual defects have been inspected.

## Scientific

* [ ] The six-stage narrative is coherent.
* [ ] Mass-dependent evolutionary pathways are represented appropriately.
* [ ] Calculated quantities are distinguished from artistic effects.
* [ ] Numerical quantities have defined meanings and units where applicable.
* [ ] Scientific assumptions and limitations are documented.
* [ ] Unsupported scientific claims have been removed.

## Engineering

* [ ] Build and run instructions are documented.
* [ ] Dependencies are documented.
* [ ] Required assets are included or properly accounted for.
* [ ] Resource management has been reviewed.
* [ ] Relevant tests have been run.
* [ ] The application has been executed and visually inspected.
* [ ] Known limitations are documented honestly.

A checkbox must not be marked complete without supporting evidence.

# 16. Change Control

Any proposed change that materially affects scope, architecture, scientific interpretation, or required interaction must be explained before implementation.

Small implementation decisions may be made autonomously when they preserve the approved requirements.

If a requirement is infeasible, propose a specific alternative and explain the trade-off.

Do not silently remove required features to make implementation easier.

Do not expand the scope without a clear benefit.

# 17. Final Product Standard

STELLARIS must communicate a scientific journey through a carefully designed interactive visual experience.

It must combine:

* A clear scientific narrative.
* Meaningful Computer Graphics techniques.
* Cinematic visual presentation.
* Reliable user interaction.
* Scientifically defensible simplifications.
* Reproducible execution.
* Evidence-based verification.

**The measure of success is the quality, correctness, and demonstrability of the working application—not the number of files, lines of code, effects, or features generated.**

---

# 18. Controlled Expansion Specification

**Status:** Approved design direction for consideration and staged implementation. The features in this section are not automatically current capabilities. The existing implementation status remains the one recorded in `GRAPHICS_TECHNIQUES.md`, `SCIENCE_NOTES.md`, `DEVELOPMENT_LOG.md`, and `VERIFICATION.md`.

## 18.1 Expansion objective

Extend STELLARIS from a polished single-star cinematic sequence into a more capable stellar-formation and evolution visualisation. The extension must improve at least one of these dimensions: scientific interpretability, environmental complexity, distinct physical phenomenon representation, observational authenticity, or meaningful user exploration.

Do not expand scope by merely increasing glow, random particle counts, texture resolution, or number of unrelated space objects. A proposed addition must connect to the existing molecular-cloud → collapse → protostar → accretion → main-sequence → final-outcome narrative.

The existing six-stage single-star sequence is the default and must remain fully functional. New modes are optional branches or inspection layers, not replacements for the core experience.

## 18.2 Priority classes

| Class | Scope | Acceptance policy |
|---|---|---|
| P0 — Core visual/technical improvement | Cloud structure, volumetric quality, layered stellar rendering, particle taxonomy, deterministic inspection | Required before broad feature expansion; each item must pass its own visual and performance checks |
| P1 — Distinctive scientific visualisation | Magnetic-field overlays, structured remnant morphology, scientific inspection and cross-sections, attributed observation mode | Implement after the P0 systems are stable |
| P2 — Scenario expansion | Binary formation, clustered formation, triggered compression, mass comparison laboratory, multi-band observation | Implement only as integrated presets/workflows using shared systems |
| P3 — Experimental | Temporal reconstruction, advanced multiple-scattering approximation, more elaborate procedural instability, optional compute-updated particles | Prototype, profile and validate independently; do not make the core experience depend on them |

If the target GPU cannot sustain the feature at the intended quality, prefer a lower-cost implementation or defer the feature. Do not compromise the reliability of the default application to meet the feature count.

## 18.3 Molecular-cloud environment expansion (P0)

The cloud must have a recognisable large-scale structure and internal hierarchy. The target appearance is a complex molecular-cloud region, not a spherical fog shell.

Required visual building blocks:

- broad cloud envelope with an irregular silhouette and depth variation;
- elongated filaments and ridges that connect or lead toward dense regions;
- at least one visually legible dense core or condensation;
- lower-density voids and dust lanes that create contrast;
- optional compact globules and smaller cores;
- controllable external illumination and local scattering/emission;
- deterministic evolution of the cloud's dominant structure during collapse.

Each contribution must be independently adjustable. Use seeded procedural fields and a limited number of scales. High-frequency details must not dominate the silhouette or flicker at normal viewing distance.

Acceptance: the cloud reads as a three-dimensional structured volume at a wide shot and a close shot; the core is visually identifiable; filaments remain stable when seeking; the volume does not turn into uniform glowing smoke; depth-aware compositing does not leak through opaque objects; the chosen quality setting meets the target performance budget.

## 18.4 Advanced volumetric rendering (P0/P3)

Improve the existing volume shader incrementally. The implementation must retain correct transmittance/opacity behavior and its current premultiplied compositing contract.

Potential techniques, in order of evaluation:

1. density-aware/adaptive ray steps with bounded minimum/maximum step sizes;
2. depth-aware upsampling of half-resolution volume results;
3. improved shadow sampling and reduced banding;
4. stable multi-scale density composition and anti-aliasing;
5. a bounded ambient or multiple-scattering approximation;
6. optional temporal accumulation with explicit history invalidation.

Do not jump directly to path tracing or a complete radiative-transfer solver. The project requires a real-time, understandable approximation. Explain the energy/brightness implications of each added term and test the effect at different exposure settings.

Acceptance: no visible step banding at normal camera distances; no volume halos bleeding across the stellar silhouette; no severe temporal ghosting; no unexplained brightness changes after seek; no unacceptable cost increase in the default profile.

## 18.5 Stellar atmosphere and surface activity (P0/P1)

Represent the star as several visually distinct layers:

- **Photosphere:** the principal emitting surface, with blackbody-informed colour, limb darkening, controlled granulation-like pattern, and optional cooler starspot regions.
- **Chromosphere:** a thin emission/transition layer whose visibility is controlled by the scene and camera.
- **Corona:** extended low-density outer emission with asymmetric/structured variation rather than a uniform spherical aura.
- **Prominences and magnetic loops:** curved, localised structures attached to selected surface regions.
- **Flares:** deterministic, localised, time-bounded bursts with a surface origin and restrained HDR response.
- **Optional coronal mass ejection:** an outward-moving shell/particle event with explicit prescribed/illustrative status.

A different shader/material is justified when the optical role differs. Do not create one shader per cosmetic variant. Maintain a shared parameter interface where practical.

Scientific boundary: granulation-like cells, spots, loops, flares and ejections are visual representations unless driven by a separate validated physical model. Do not say that the application solves convection, plasma dynamics, magnetic reconnection or MHD.

Acceptance: each layer is identifiable at the intended viewing distance; the layers do not create a flat white sphere or overexposed halo; surface details do not swim unnaturally when seeking; the object remains legible under the existing HDR/tone-mapping pipeline; the UI labels the relevant approximations.

## 18.6 Typed particle phenomena (P0/P1)

Expand the existing particle system through distinct particle species rather than independent ad hoc systems. Candidate species:

| Species | Narrative role | Required trajectory character |
|---|---|---|
| Collapse/infall | Material moving toward the centre | Inward, convergent and time-bounded |
| Accretion disc | Orbiting material | Differential Keplerian-inspired orbital motion |
| Protostellar jet knots | Bipolar outflow | Collimated, outward, structured knot progression |
| Stellar wind | Continuous mass outflow | Radial or latitude-dependent outward flow |
| Flare/CME ejecta | Localised surface event | Surface-originating, directional expanding structure |
| Planetary-nebula shell | Envelope shedding | Expanding shell with layered/anisotropic distribution |
| Supernova ejecta | Explosion aftermath | Fast, asymmetric, stratified expansion |
| Ambient dust | Environmental depth | Low-contrast, slow, non-distracting motion |

Not all species need to be active in every scene. Each must be identifiable by trajectory, spatial origin, timing and visual treatment. Preserve stable IDs/seeds so the same timestamp reproduces the same arrangement.

Acceptance: particles do not flicker, pop, form obvious spokes or all move identically; their apparent size/brightness is controlled at distance; species can be inspected or toggled without changing the underlying scenario; particle count and GPU cost are documented.

## 18.7 Magnetic-field visualisation (P1)

Add magnetic-field lines as an optional explanatory overlay for stellar activity, jets or a selected cloud/disc example.

Minimum useful implementation:

- one explicitly documented analytical field model, beginning with a dipole field for a stellar case;
- deterministic line seeding and curve integration;
- bounded line count/length and clear termination rules;
- subtle line/ribbon rendering with depth-aware fading;
- optional tracer particles that follow the selected field geometry;
- UI explanation of the model and its limitations.

Do not imply that this field controls or launches the existing jets unless the model explicitly couples them. Analytic lines over an artistically animated jet remain a visual explanation, not a self-consistent magnetohydrodynamic solution.

Acceptance: the field orientation is mathematically consistent with the implemented vector field; line geometry is stable under camera changes; visibility is controllable; labels explicitly say analytic/prescribed; rendering does not obscure the primary object.

## 18.8 Supernova and remnant structure (P1)

The high-mass pathway may be extended with:

- a distinguishable shock-front boundary;
- inner and outer ejecta regions;
- asymmetry in expansion and brightness;
- filamentary/clumpy structures;
- optional Rayleigh–Taylor-like finger morphology at shell interfaces;
- a time-evolving remnant with fading emission and persistent irregular structure.

The morphology may be generated procedurally with deterministic seeds. It must not be called a solved Rayleigh–Taylor instability or a hydrodynamic supernova simulation. Keep the existing mass-pathway qualification and never show core-collapse supernovae for the white-dwarf pathway.

Acceptance: the sequence communicates shock propagation and ejecta structure; the remnant is not a perfectly smooth expanding sphere; the visual morphology is deterministic; all pathway and timing rules remain correct.

## 18.9 Scientific inspection mode (P1)

Add a focused inspection mode that exposes the model rather than adding a dense cockpit HUD. The user should be able to toggle only relevant overlays for the selected stage:

- normalized/procedural density field;
- temperature reference or prescribed temperature field;
- velocity direction/magnitude when available;
- particle trajectories by species;
- analytic magnetic-field lines;
- a plane or cross-section through the volume;
- object and stage annotations;
- explanatory model/limitation information.

Every view must state whether the displayed quantity is calculated, parameterized, prescribed, normalized, reference, or artistic. Do not display fabricated SI values from a normalized procedural field. A cross-section of a shader density field must sample the same field used by the volume renderer.

Acceptance: each overlay corresponds to an actual model/render input; the display includes units or a clear normalized label; the user can turn it off; the cinematic mode remains visually clean; cross-section orientation and scale are understandable.

## 18.10 Authentic observation and multi-wavelength mode (P1/P2)

Integrate a small, curated set of authentic observations relevant to the project's actual stages: molecular clouds/star-forming regions, protostellar jets/discs, stellar surfaces/solar activity, planetary nebulae, supernova remnants and compact-object environments.

For each asset, retain source institution, original page/identifier, creator/credit, usage terms, wavelength/filter/instrument, original resolution, image processing, local derivative, checksum and intended use. Verify the asset-specific terms; do not assume blanket rights based on institution name.

The observation workflow should support:

1. selecting a real observation/reference;
2. viewing the authentic image with its source and band metadata;
3. viewing a related STELLARIS procedural scene;
4. comparing the two while explicitly describing the rendering as a conceptual visualization rather than a reconstruction.

Multi-wavelength presentation must use correctly identified band images/data when available. Do not simulate infrared/X-ray/radio observations by merely tinting a visible-light image. For a synthetic band view, document the emission/absorption approximation and label it synthetic.

Acceptance: no asset appears without provenance; the wavelength/band label matches the source; false-colour/composite imagery is identified as processed; source imagery is not represented as a 3D measured volume; the project respects source usage and credit requirements.

## 18.11 Formation-scenario expansion (P2)

The default remains the existing single-star formation path. Add optional scenario presets only after they can reuse the core cloud, volume, particle, camera, timeline and annotation systems.

| Scenario | What changes in the setup | What the application may claim |
|---|---|---|
| Low-/intermediate-mass star | Mass and relevant final pathway | A representative parameterized pathway |
| Massive star | Higher-mass parameter range, stronger feedback visuals, massive-star outcome branch | A representative massive-star pathway subject to model limits |
| Binary formation | Two dense condensations, two protostellar objects, prescribed orbital/circumbinary structure | An illustrative binary-formation configuration, not a solved fragmentation calculation |
| Clustered formation | Multiple cloud cores and stars at staggered stages | A procedural cluster scenario, not N-body cluster evolution |
| Triggered compression | An incoming shock/compression front and a denser region after contact | A visual model of compression-triggered collapse, not proof of a real trigger event |

A scenario configuration must be deterministic and reproducible. Provide a clear distinction between the current single-star narrative and optional multi-object examples.

## 18.12 Comparative stellar evolution laboratory (P2)

Provide a way to compare a small number of representative initial masses using the existing stellar model or a separately integrated, attributed reference-track dataset. Comparison dimensions may include:

- luminosity and effective temperature trends;
- radius trend;
- main-sequence lifetime order of magnitude;
- representative stage sequence;
- final outcome category and uncertainty;
- timeline alignment (presentation time versus physical timescale).

Do not imply that a short normalized presentation timeline represents real stellar lifetime. If published evolutionary tracks are used, preserve dataset version, source, interpolation method, units and limitations. Do not mix a published track with the existing simplified formula without identifying which curve is which.

Acceptance: each curve has a named model/source; units and scaling are clear; uncertain boundaries are visible; comparison does not silently alter the default timeline; all interpolated values stay within the supported data domain.

## 18.13 Quality tiers and performance envelope

The default interactive quality must target stable operation at 1920×1080 on the reference RTX 4050 Laptop GPU (6 GB VRAM), subject to measurement after each feature is added. Define at least:

- **Performance:** reduced volume resolution/steps and fewer particles/overlays;
- **Balanced:** default interactive balance;
- **High:** higher volume and particle quality, still intended for interactive use;
- **Capture:** deterministic high-quality screenshots, permitted to run slower.

Quality settings alter rendering cost only. They must not alter scientific calculations, scenario parameters, stage timing, pathway selection or the deterministic scene state.

Measure frame-time distributions and GPU pass times in representative worst-case scenes. Track memory use for volumes, textures, geometry, particle buffers and post-process targets. A 4K image is not automatically preferable to a well-filtered 2K asset; choose source resolution by screen coverage and usage.

## 18.14 Global expansion acceptance gate

The expansion is acceptable only when all applicable items below are evidenced:

- [ ] Existing six-stage single-star sequence still builds, launches and plays.
- [ ] Existing seek, rewind, reset, speed and stage selection remain deterministic.
- [ ] New cloud morphology remains stable at arbitrary timeline positions.
- [ ] Volume improvements pass edge, banding, aliasing and compositing inspection.
- [ ] Stellar atmospheric layers remain distinct and controlled under HDR.
- [ ] Particle species have distinct, repeatable motion and do not cause excessive overdraw.
- [ ] Magnetic lines use an explicitly identified analytic/prescribed field model.
- [ ] Supernova morphology does not imply a full hydrodynamic calculation.
- [ ] Scientific overlays show real underlying fields and correct provenance/units.
- [ ] Observation assets have complete, verified provenance and appropriate credit.
- [ ] Optional scenarios do not break the default scenario or its pathway rules.
- [ ] The comparative laboratory identifies the source/model for every displayed track.
- [ ] Performance is measured on the target GPU at a stated resolution and quality preset.
- [ ] No feature is described as implemented until it has passed its relevant verification.
