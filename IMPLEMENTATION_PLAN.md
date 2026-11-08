# STELLARIS — Implementation Plan

## 1. Purpose

This document defines the development sequence, milestone gates, verification procedures, and completion criteria for STELLARIS.

The objective is to develop a polished, interactive, scientifically defensible 3D visualization of stellar evolution without sacrificing architectural quality, visual fidelity, scientific accuracy, or reliability.

**Development principle:** Establish a verified foundation first, then build the application incrementally. Every milestone must produce a testable result before the next major milestone begins.

Claude must read `CLAUDE.md`, `PROJECT_SPEC.md`, `VISUAL_DESIGN.md`, and `SCIENTIFIC_MODEL.md` before planning or implementing project work.

---

## 2. Non-Negotiable Development Rules

1. Do not begin full application implementation before the architecture and graphics technology have been selected.
2. Do not assume that a particular graphics library, game engine, or rendering API satisfies the faculty's academic requirements.
3. Verify the faculty's requirement concerning GLUT/freeGLUT before finalizing the architecture.
4. Do not generate the entire application in one large implementation pass.
5. Do not treat successful compilation as proof that the application works correctly.
6. Do not claim that a feature works unless it has been implemented and verified.
7. Do not fabricate build results, test results, screenshots, performance measurements, or scientific references.
8. Do not introduce unnecessary dependencies, frameworks, abstractions, or features.
9. Do not replace the project's scientific scope with a full astrophysics simulation.
10. Do not sacrifice the visual quality defined in `VISUAL_DESIGN.md` merely to finish implementation faster.
11. Preserve deterministic timeline behavior. A given timeline position must reproduce the same intended visual state, subject to normal hardware and rendering differences.
12. Keep the project buildable throughout development wherever practical.
13. Use the installed development plugins when they provide a concrete benefit; do not invoke them mechanically.
14. Explain consequential technical decisions in language suitable for a developer who is still learning.
15. Keep routine implementation work moving without repeatedly asking the user to approve trivial details. Ask when a decision materially changes architecture, academic compliance, scope, or project direction.

---

## 3. Phase 0 — Requirements and Environment Verification

### Objective

Resolve critical unknowns before selecting the architecture or writing application code.

### 3.1 Inspect the project

Read all four existing project documents.

Confirm that:

* The project directory is clean.
* No unrelated source code or old project configuration is being reused.
* The six required stellar-evolution stages are understood.
* The visual and scientific requirements are understood.
* The acceptance criteria are identifiable and testable.

Report any contradictions or missing requirements instead of silently resolving them.

### 3.2 Verify the academic graphics requirement

Determine exactly what the faculty requires concerning GLUT/freeGLUT.

Clarify whether the requirement means:

* GLUT/freeGLUT must be used by the main application.
* GLUT/freeGLUT must be demonstrated in a separate program.
* GLUT/freeGLUT is one permitted option among several.
* A particular set of OpenGL or graphics-programming techniques must be demonstrated.

Do not treat these interpretations as interchangeable.

If the requirement cannot be verified from the available material, identify it as unresolved and obtain clarification before making an irreversible architecture decision.

### 3.3 Audit the development environment

Inspect the actual Windows development environment before installing or configuring dependencies.

Verify the availability and versions of relevant tools, including:

* C++ compiler and toolchain, if a native C++ architecture is selected.
* Build system.
* Git.
* Graphics drivers and supported graphics capabilities.
* Required SDKs and libraries for the selected architecture.

Do not assume that a tool is installed merely because it is commonly used.

Avoid unnecessary system-wide changes. Explain any installation or configuration that requires significant privileges or modifies the user's environment.

### Phase 0 exit criteria

* All four project specifications have been reviewed.
* The GLUT/freeGLUT requirement is clarified or explicitly recorded as unresolved.
* The development environment has been inspected.
* Important constraints and unknowns are documented.
* No full application implementation has started.

---

## 4. Phase 1 — Architecture and Technology Decision

### Objective

Choose a graphics architecture that satisfies the academic requirement and supports the intended visual quality.

### 4.1 Evaluate viable options

Evaluate suitable options against the following criteria:

| Criterion             | Required evaluation                                                                    |
| --------------------- | -------------------------------------------------------------------------------------- |
| Academic compliance   | Does the architecture satisfy the verified faculty requirement?                        |
| Visual capability     | Can it produce the intended cinematic 3D environment?                                  |
| Interaction           | Can it support timeline seeking, camera controls, and interactive scientific controls? |
| Performance           | Is it appropriate for the target Windows laptop and RTX 4050 Laptop GPU?               |
| Development effort    | Can the application be implemented and maintained within the project's scope?          |
| Build reliability     | Can the project be built and run reproducibly?                                         |
| Debugging             | Can rendering, timing, and interaction problems be diagnosed effectively?              |
| Dependency management | Are dependencies manageable and documented?                                            |
| Demonstration         | Can the application be demonstrated reliably in an academic setting?                   |
| Extensibility         | Can the visual system and six stages be refined without extensive rewrites?            |

Do not choose a technology solely because it is popular, familiar, or theoretically powerful.

### 4.2 Establish the architecture

Once the requirements and technology are verified:

1. Select the rendering framework and graphics API.
2. Select the programming language and language standard.
3. Define the build system and dependency-management strategy.
4. Define the rendering architecture.
5. Define the timeline and animation architecture.
6. Define scene organization and stage transitions.
7. Define input handling and user-interface integration.
8. Define resource-loading and asset-management responsibilities.
9. Define logging, error handling, and testing.
10. Record the rationale and important trade-offs.

The final choices must be based on the verified constraints and actual tool availability.

### 4.3 Architecture decision gate

Before substantial implementation, present the user with a concise architecture decision record containing:

* Selected technology stack.
* Why it satisfies the academic requirement.
* Why it supports the visual objectives.
* Major dependencies.
* Important limitations and risks.
* Expected build and run workflow.
* Alternatives considered and why they were rejected.

Obtain confirmation for this consequential decision before proceeding.

### Phase 1 exit criteria

* The technology stack is explicitly documented.
* The academic requirement is addressed.
* The architecture has a clear rationale.
* The dependency and build strategy is defined.
* The architecture decision has been confirmed.

---

## 5. Phase 2 — Minimal Working Foundation

### Objective

Establish a small, reliable application that can be built, launched, and inspected.

### Implementation order

1. Create the agreed directory structure.
2. Configure the build system.
3. Integrate the minimum required dependencies.
4. Create the application entry point.
5. Initialize the graphics context and window.
6. Implement the main application loop.
7. Implement basic input handling.
8. Establish resource and error-handling conventions.
9. Implement clean shutdown.
10. Add the minimum diagnostic output required for debugging.

Do not add all six stellar stages at this point.

### Verification

* Configure and build the project from a clean state.
* Launch the application.
* Confirm that the graphics context initializes successfully.
* Confirm that the application responds to input.
* Confirm that it exits cleanly.
* Check for startup errors and graphics initialization failures.
* Document the exact commands needed to configure, build, and run it.

### Phase 2 exit criteria

A minimal working application runs reliably on the target Windows environment.

---

## 6. Phase 3 — Rendering Foundation

### Objective

Create the rendering capabilities required by the visual design before building the complete stellar-evolution sequence.

Implement only the rendering systems justified by the selected architecture.

Potential systems include:

* Camera and view controls.
* Perspective projection.
* Shader compilation and error reporting.
* Mesh and geometry management.
* Materials and lighting.
* Particle rendering.
* Procedural geometry.
* Transparency and depth handling.
* Post-processing, where supported and justified.
* Time-based animation.
* Resource cleanup and GPU-resource management.

Not every listed system must be implemented as a separate abstraction. Prefer the simplest architecture that supports the required behavior.

### Verification

Create a small rendering test scene that demonstrates the actual capabilities required by STELLARIS.

Evaluate:

* Depth and spatial readability.
* Lighting quality.
* Material response.
* Particle behavior.
* Camera movement.
* Animation stability.
* Rendering performance.
* Shader and resource errors.

Do not use a technically functioning but visually weak test scene as evidence that the final visual requirements have been satisfied.

### Phase 3 exit criteria

The rendering foundation is stable, its limitations are understood, and it can support the intended visual direction.

---

## 7. Phase 4 — Timeline and Application State

### Objective

Implement the temporal foundation before constructing the complete sequence.

The timeline is a central system, not an afterthought.

### Required capabilities

Implement:

* Play.
* Pause.
* Resume.
* Reset.
* Forward seeking.
* Backward seeking.
* Timeline scrubbing.
* Playback-speed adjustment.
* Stage selection, where appropriate.
* Consistent mapping between timeline position and scene state.

### Determinism requirements

The scene must not depend exclusively on accumulated frame-by-frame changes.

Where practical, calculate animation from a canonical timeline position and stable procedural inputs.

For example, if the user seeks to a particular time, the application should reconstruct the intended state for that time rather than relying on the exact sequence of frames previously rendered.

Verify that:

* Seeking backward works.
* Seeking forward works.
* Pausing and resuming work.
* Reset returns to the defined initial state.
* Repeatedly seeking to the same time produces a consistent state.
* Stage boundaries do not introduce unintended visual discontinuities.
* Changing playback speed affects playback rate rather than corrupting the timeline state.

### Phase 4 exit criteria

Timeline controls work reliably and all major scene systems can use the same temporal model.

---

## 8. Phase 5 — Build the Six Stellar-Evolution Stages

### Objective

Implement the required stages in a controlled sequence, preserving continuity and visual quality.

Use `PROJECT_SPEC.md`, `VISUAL_DESIGN.md`, and `SCIENTIFIC_MODEL.md` as the authoritative requirements.

### Stage 1 — Molecular Cloud

Implement:

* A structured, volumetric-looking molecular cloud.
* Meaningful depth and density variation.
* Coherent spatial composition.
* Appropriate color, lighting, and motion.
* A recognizable initial state.

Avoid an undifferentiated sphere of uniformly distributed particles.

**Gate:** The cloud reads as a structured astrophysical environment from the intended camera angles.

### Stage 2 — Gravitational Collapse

Implement:

* A clear transition from the initial cloud.
* A visually legible concentration of material.
* Coherent inward motion.
* A relationship between the timeline and the degree of collapse.

Avoid arbitrary particle motion that merely looks turbulent without communicating collapse.

**Gate:** The change in density and structure clearly communicates the intended process.

### Stage 3 — Protostar Formation

Implement:

* A central forming object.
* Appropriate luminosity and color development.
* Surrounding material consistent with the simplified scientific model.
* A transition that connects the collapse to protostar formation.

**Gate:** The protostar is visually distinguishable from the surrounding cloud and its formation is understandable.

### Stage 4 — Accretion Disc and Bipolar Jets

Implement:

* A clearly defined accretion disc.
* Appropriate spatial orientation.
* A central luminous object.
* Bipolar jets aligned consistently with the chosen disc axis.
* Motion and lighting that preserve the readability of the structure.

Do not add physically misleading features merely for visual spectacle.

**Gate:** The disc, central object, and bipolar jets remain visually legible and spatially coherent.

### Stage 5 — Main-Sequence Star

Implement:

* A recognizable stellar body.
* Appropriate stellar color and surface appearance.
* A clear transition from the protostellar stage.
* Any explanatory labels or simplified properties supported by the scientific model.

Do not imply that the visual timescale represents the actual duration of stellar evolution.

**Gate:** The star reads clearly as a stable main-sequence stage within the project's simplified timeline.

### Stage 6 — Mass-Dependent Stellar Fate

Implement the applicable outcomes described in `SCIENTIFIC_MODEL.md`, including:

* White dwarf formation.
* Neutron star formation.
* Black hole formation.

Represent these as mass-dependent pathways rather than implying that every star follows one universal sequence.

The interface must make the selected pathway understandable. If the user can select an initial stellar mass or another pathway control, ensure that the control has a clearly defined effect on the visualization.

Do not invent universal mass thresholds or unsupported scientific details.

**Gate:** Each supported outcome is visually distinct, scientifically defensible within the project's simplified model, and connected coherently to the preceding stages.

### Cross-stage acceptance criteria

* All six stages are accessible.
* The timeline and stage selection remain consistent.
* Stage transitions are deliberate and understandable.
* Camera behavior does not obscure the subject.
* Visual quality is consistent across stages.
* Seeking to any supported point reproduces the appropriate state.
* Stage-specific scientific explanations do not contradict the scientific model.

---

## 9. Phase 6 — Scientific Information and Interaction

### Objective

Make the visualization educational and interactive without overwhelming the scene.

Implement the controls and information panels defined in `PROJECT_SPEC.md`.

Potential elements include:

* Current stage name.
* Concise stage description.
* Timeline position.
* Playback state and speed.
* Relevant physical quantities and units.
* Stage-specific explanatory annotations.
* Supported scientific controls.

### Requirements

* Every control must have a defined purpose.
* Displayed numerical values must have documented meanings and units.
* Approximate or illustrative values must be identified as such.
* Labels must not imply precision beyond the model.
* Scientific explanations must remain consistent with `SCIENTIFIC_MODEL.md`.
* Interface elements must remain readable against the rendered scene.
* Controls must not obstruct the most important visual structures.

### Verification

Test every control individually and in combination with timeline seeking, pause/resume, reset, and stage transitions.

### Phase 6 exit criteria

The application communicates both the visual sequence and the scientific meaning of its stages without making unsupported claims.

---

## 10. Phase 7 — Visual Refinement

### Objective

Bring the complete application to the intended cinematic quality.

Visual refinement begins after the core rendering and timeline systems are stable. It must not be postponed until the final hours of development.

### Refinement order

1. **Composition:** Establish strong framing, subject placement, and visual hierarchy.
2. **Major structures:** Improve the cloud, protostar, disc, jets, star, and remnants.
3. **Materials and lighting:** Improve depth, shape readability, and separation between elements.
4. **Motion:** Refine the timing, direction, scale, and continuity of animation.
5. **Atmosphere:** Add restrained supporting effects that strengthen the scene.
6. **Camera:** Refine movement, transitions, framing, and viewing angles.
7. **Interface:** Improve typography, spacing, hierarchy, feedback, and legibility.
8. **Transitions:** Eliminate abrupt or unintended changes between stages.
9. **Performance:** Optimize expensive effects without needlessly degrading visual quality.
10. **Final polish:** Inspect the complete experience rather than isolated screenshots.

### Visual review protocol

For each major refinement:

1. Build the application.
2. Run it.
3. Inspect the actual rendered result.
4. Compare the result against `VISUAL_DESIGN.md`.
5. Identify the largest remaining visual weakness.
6. Make a focused correction.
7. Rebuild and inspect again.

Prioritize improvements by their visual impact. Do not spend substantial time polishing minor particles or interface details while major composition or structural problems remain.

### Phase 7 exit criteria

The complete application has been visually inspected, and no known major defect undermines the intended presentation.

---

## 11. Phase 8 — Testing and Reliability

### Objective

Verify functional correctness, scientific consistency, visual stability, and reproducibility.

### 11.1 Functional tests

Test:

* Application startup and shutdown.
* Every playback control.
* Timeline scrubbing.
* Forward and backward seeking.
* Reset behavior.
* Playback-speed changes.
* Stage selection.
* All supported scientific controls.
* Interface updates.
* Error handling.
* Resource cleanup.

### 11.2 Timeline tests

Test:

* The initial timeline position.
* Stage boundaries.
* Intermediate positions within each stage.
* Backward seeking across a stage boundary.
* Forward seeking across a stage boundary.
* Repeated seeking to the same position.
* Reset after extensive interaction.
* Pause and resume at different positions.
* Behavior when playback reaches the end of the timeline.

### 11.3 Rendering tests

Inspect:

* Missing or incorrectly compiled shaders.
* Invalid or missing resources.
* Flickering and z-fighting.
* Incorrect transparency ordering.
* Camera clipping.
* Unexpected visual discontinuities.
* Particle or geometry artifacts.
* Memory or GPU-resource leaks.
* Rendering failures after repeated resets or stage changes.

### 11.4 Scientific review

Check:

* Stage descriptions against the scientific model.
* Mass-dependent evolutionary outcomes.
* Numerical values and units.
* Claims about stellar formation and evolution.
* The distinction between visual approximation and physical simulation.
* The absence of unsupported precision or fabricated references.

### 11.5 Performance tests

Evaluate the application on the target Windows laptop under representative conditions.

Inspect:

* Startup time.
* Responsiveness.
* Frame-time consistency.
* GPU and CPU utilization.
* Memory usage.
* Performance during the most visually expensive stages.
* Performance after extended playback and repeated seeking.

Record actual measurements when available. Do not invent a frame rate or claim that a performance target has been met without testing.

### 11.6 Clean-build verification

Confirm that the project can be built from its documented instructions without depending on accidental local state, undocumented files, or stale generated artifacts.

### Phase 8 exit criteria

Critical functionality is tested, known defects are documented, and the application is stable enough for a complete academic demonstration.

---

## 12. Phase 9 — Documentation and Demonstration Preparation

### Objective

Prepare a reproducible project that can be built, explained, and demonstrated.

Document:

* Project purpose and scope.
* Selected technology stack and rationale.
* Dependencies and setup.
* Build and run instructions.
* Controls.
* Timeline model.
* Rendering techniques used.
* Scientific approximations and limitations.
* Testing performed.
* Known limitations.
* Academic graphics requirements and how the implementation addresses them.

Ensure that documentation describes the actual implementation, not an intended or hypothetical future state.

Prepare a demonstration sequence that showcases:

1. The initial molecular cloud.
2. Gravitational collapse.
3. Protostar formation.
4. The accretion disc and bipolar jets.
5. The main-sequence star.
6. The mass-dependent stellar outcomes.
7. Timeline scrubbing and playback controls.
8. The educational information and scientific controls.

The demonstration must work on the intended machine without relying on internet connectivity unless a documented dependency genuinely requires it.

---

## 13. Milestone Management

At the end of every milestone, Claude must provide a concise status report containing:

### Completed

What was actually implemented.

### Verified

What was built, run, tested, or visually inspected.

### Not yet verified

Anything that exists in code but has not been confirmed to work.

### Known issues

Defects, limitations, or unresolved questions.

### Files changed

The important files created or modified.

### Next milestone

The next concrete development task and its acceptance criteria.

A milestone is not complete merely because its code has been written.

If a verification step cannot be performed, state the limitation and explain the available alternative. Do not represent an unverified result as successful.

---

## 14. Git and Change Management

Use Git to preserve meaningful project milestones.

1. Initialize version control once the clean project directory and basic project files are ready.
2. Add an appropriate `.gitignore` before committing generated files.
3. Keep source code and required project configuration under version control.
4. Avoid committing build output, temporary files, caches, or unnecessary large generated assets.
5. Make logical commits at stable milestones when practical.
6. Review significant changes before committing.
7. Never discard unrelated user work or overwrite important files without checking their purpose.
8. Do not run destructive cleanup commands without confirming their scope.
9. Do not store credentials or secrets in the repository.

The repository should make it possible to identify which milestone introduced a regression.

---

## 15. Efficient Use of Claude Code and Plugins

Use the available tools where they improve correctness or efficiency.

### Context7

Consult current documentation when a technical decision depends on the actual APIs, configuration, or behavior of a selected library.

Do not rely on remembered API details when the installed version may differ.

### Feature development

Use structured feature-development workflows for significant features that involve multiple components or architectural changes.

Keep tasks scoped to a clear deliverable and acceptance criteria.

### Review toolkit

Use code review to identify correctness problems, security issues, edge cases, maintainability concerns, and potential regressions.

Review findings must be assessed against the actual project requirements.

### Code simplification

Use code simplification after behavior is established and tests provide a basis for detecting regressions.

Do not simplify working code in a way that weakens visual quality, determinism, scientific clarity, or maintainability.

### General operating rule

Use parallel work only when tasks are genuinely independent and the integration cost is justified. Avoid having multiple agents modify the same core systems simultaneously.

Do not start long-running autonomous work without bounded tasks, explicit acceptance criteria, and a way to verify the result.

---

## 16. Scope and Quality Trade-Offs

When time or performance constraints require trade-offs, prioritize in this order:

1. Academic compliance.
2. Correct application architecture.
3. Functional correctness and timeline determinism.
4. Scientific defensibility.
5. Core visual quality.
6. Reliable interaction and usability.
7. Performance and resource stability.
8. Secondary visual effects and optional enhancements.

This ordering does not mean that visual quality is optional. It means that secondary effects should be reduced before the central experience is compromised.

If a requested effect is too expensive, first investigate a more efficient implementation. If a trade-off is unavoidable, document it and explain its visual consequences.

Do not silently remove required stages, controls, or scientific behavior to make development easier.

---

## 17. Final Acceptance Gate

STELLARIS is ready for final delivery only when all applicable criteria below have been verified.

### Functional

* All six required stages are implemented.
* Timeline controls behave consistently.
* Stage transitions are coherent.
* Supported scientific controls work.
* The application launches and exits reliably.

### Scientific

* The visualization follows `SCIENTIFIC_MODEL.md`.
* Numerical values and units are meaningful.
* Mass-dependent outcomes are represented appropriately.
* Simplifications are documented.
* No unsupported claims are presented as established facts.

### Visual

* The application follows `VISUAL_DESIGN.md`.
* Major astrophysical structures are visually recognizable.
* Lighting, depth, composition, and motion work together.
* Transitions and camera behavior are coherent.
* The complete experience has been visually inspected.

### Engineering

* The project builds from documented instructions.
* Dependencies are documented.
* Important errors are diagnosable.
* Resources are managed appropriately.
* No known critical defect remains unresolved.
* The repository does not depend on undocumented local state.

### Academic demonstration

* The verified faculty requirement is satisfied.
* The graphics techniques used can be explained.
* The implementation's scientific limitations can be explained.
* The demonstration sequence works reliably.

### Delivery

* Setup, build, and run instructions are complete.
* Controls and limitations are documented.
* Testing claims reflect actual verification.
* The delivered project corresponds to the documented implementation.

A criterion that has not been tested must be marked **unverified**, not passed.

---

## 18. First Implementation Session

When implementation begins, do not immediately generate the full application.

Perform these steps in order:

1. Read the four project documents.
2. Inspect the project directory.
3. Identify unresolved requirements.
4. Verify the faculty's GLUT/freeGLUT requirement.
5. Audit the development environment.
6. Evaluate viable architecture options.
7. Present the architecture decision record.
8. Wait for confirmation of the architecture.
9. Establish the minimal working build.
10. Verify it before proceeding to the rendering foundation.

**The first technical milestone is a verified architecture and build foundation, not a visually elaborate but untested application.**

---

## 19. Controlled expansion roadmap — continue from the current application

This roadmap is a staged extension of the implementation already recorded in `DEVELOPMENT_LOG.md` and `VERIFICATION.md`. It is not a new-project bootstrap plan. Do not repeat completed environment setup or replace working systems without a demonstrated need. Start by auditing the actual current source against the baseline results, because later code changes may have changed the state since the last recorded verification.

### 19.1 Entry gate: baseline audit before expansion

Before the first enhancement:

1. Check Git status and preserve the current working state.
2. Configure/build the current Release target using the existing project scripts.
3. Run `ctest` and the application's existing `--selftest`.
4. Run the existing `--benchmark` and capture representative timeline states.
5. Inspect the actual cloud, volume, particle, stellar, disc, remnant, lensing and UI implementations.
6. Record any difference between current code and the claims in the existing documentation.
7. Do not start a new feature while a regression in the default six-stage path remains unexplained.

**Exit condition:** current baseline build/test/benchmark status is known and the current default scenario is visually stable.

### 19.2 Workstream A — molecular-cloud structure (P0)

**Goal:** make the cloud feel like a large, spatially structured star-forming environment instead of a single generic volume.

Implementation order:

1. Audit the current density/noise composition and identify the controlling shader functions/uniforms.
2. Separate low-frequency envelope, mid-frequency structure, filament contribution, dense-core mask and fine detail conceptually and in code.
3. Add a deterministic cloud preset/configuration with stable seed.
4. Add at least one dominant dense core and a controlled filament network.
5. Add negative-space/dust-lane masks and optional globule silhouettes only after the basic cloud reads correctly.
6. Add collapse deformation to the existing field without causing the cloud to lose its large-scale identity abruptly.
7. Inspect wide, medium and close camera views; inspect before/after stage boundaries and direct seeks.
8. Profile the volume and ensure the added structures do not require an unreasonable number of ray steps.

**Tests:** same seed/config/time produces the same density; different seeds vary the structure; density remains bounded; no visible tiling; dense core is identifiable; filaments do not alias; volume compositing remains correct.

**Do not:** add random sprites on top of the existing cloud and call it filamentary structure; add uncalibrated SI density values; introduce a full turbulence solver.

### 19.3 Workstream B — volume-rendering refinement (P0)

Implement one measurable improvement at a time:

1. Capture current baseline at identical camera/time/exposure.
2. Profile ray-march GPU time and identify the worst stages.
3. Improve density sampling/footprint and jitter first if banding or aliasing is visible.
4. Prototype adaptive step length with strict min/max bounds and a fallback path.
5. Prototype depth-aware upsampling; test against the full-resolution depth buffer and opaque silhouettes.
6. Improve light optical-depth samples only if shadow quality is the bottleneck.
7. Evaluate a bounded ambient/multiple-scattering approximation after the base integration is stable.
8. Treat temporal accumulation as P3; add only if it provides a measurable improvement and history invalidation is complete.

**Tests:** fixed-time image comparison; seek/rewind equality; exposure sweep; opaque-object edge test; resizing/minimization test; frame-time and GPU-pass comparison. Do not accept an improvement that merely brightens the whole volume.

### 19.4 Workstream C — layered stellar rendering (P0)

Implement in this order:

1. Refine the photosphere's existing blackbody/limb-darkening treatment and make detail stable at multiple viewing distances.
2. Add low-contrast granulation-like detail only if it improves the surface at the intended camera range.
3. Add starspot masks with deterministic positions and controlled contrast.
4. Add a thin chromosphere contribution with correct depth/blending behavior.
5. Add structured corona streamers with restrained brightness.
6. Add a small number of anchored prominence/loop curves.
7. Add one deterministic flare event type; only then evaluate optional CME-like ejecta.

**Tests:** no aliasing/flicker; no overexposed white disc; surface and atmosphere remain separable; no transparent-shell sorting defects; fixed timestamp reproduces the event; existing star colour/mass dependence remains intact.

**Do not:** turn the star into a conventional roughness/metalness PBR sphere; add many glow layers without visible structure; claim magnetohydrodynamic plasma simulation.

### 19.5 Workstream D — particle species (P0/P1)

Extend the existing stateless particle system rather than replacing it initially.

1. Audit current particle ID/seed/time calculation and ensure every existing trajectory is deterministic.
2. Define a compact species/type field and shared parameter structure.
3. Refactor current infall, disc, jet and ejecta paths into explicit trajectory families while preserving their current visual output.
4. Add species-specific size, fade, opacity/blend and orientation rules.
5. Add stellar-wind or CME species only after the base families are verified.
6. Add velocity-aligned streaks only where a dot cloud does not communicate motion clearly.
7. Consider compute-shader state updates only if a real interaction requirement cannot be represented by deterministic time evaluation.

**Tests:** deterministic seed/time; no species leakage into inappropriate pathways; stable particle count; no subpixel flicker; blend correctness; GPU time and overdraw measurements.

### 19.6 Workstream E — magnetic-field overlays (P1)

1. Select a single initial field model (recommended first example: analytic stellar dipole).
2. Document the equation, coordinate convention, units/normalization, singularity handling and limits.
3. Build deterministic seed points and integrate curves with a bounded step/path length.
4. Render thin ribbons/segments with depth-aware fading.
5. Add optional tracer particles after the static lines are correct.
6. Add an explicit UI label identifying the field model as analytic/prescribed.
7. Keep the overlay disabled by default in shots where it adds clutter.

**Tests:** compare selected field vectors against hand-computed values; check line direction and termination; verify deterministic generation; test orientation changes; verify overlay does not obscure stellar surface or UI.

### 19.7 Workstream F — structured supernova and remnant (P1)

1. Audit the existing massive-star supernova sequence and ensure the white-dwarf branch is unaffected.
2. Separate shock-front, inner/outer ejecta and remnant-envelope contributions.
3. Add deterministic asymmetric expansion.
4. Add seeded clumps and low-frequency shell perturbations.
5. Add optional finger-like interface morphology only after the shell remains legible.
6. Preserve the existing pathway-specific final remnant state and text qualifications.

**Tests:** no supernova before the appropriate timeline stage; none on the white-dwarf path; all effects repeat at fixed time; shell geometry remains stable; no sudden full-frame whiteout; camera and tone mapping preserve structure.

### 19.8 Workstream G — scientific inspection and observation integration (P1)

1. Define which fields actually exist and their data provenance before building UI controls.
2. Add a compact mode switch (Cinematic / Scientific / Observation) rather than showing every control simultaneously.
3. Add only data views whose values are real inputs/outputs of the current model.
4. Implement cross-section sampling from the same procedural field used by the volume renderer.
5. Add labels for units, normalized quantities and prescribed fields.
6. Curate a small observation set with verified source, band, processing, credit, usage terms and local checksum.
7. Display observation and procedural scene side by side; label the latter as conceptual unless a validated reconstruction is present.
8. Add multi-wavelength selection only when the correct band products exist; do not create false bands by tinting a single image.

**Tests:** UI values map to the actual model field; cross-section agrees with volume samples; no physical units on normalized data; source link/credit is present; band labels match metadata; asset can be loaded offline from the packaged location.

### 19.9 Workstream H — optional formation scenarios and comparative laboratory (P2)

Implement scenarios as data configurations that reuse the common renderer and timeline:

- massive-star preset;
- binary condensation/protostar preset;
- limited clustered-core preset;
- triggered-compression concept preset.

Start with one scenario at a time. Ensure the default single-star scenario remains the entry point. For a binary, add a two-subject camera composition and a prescribed/analytical orbit. For a cluster, limit object count and retain a clear focal subject. For triggered compression, show the incoming front and resulting density structure without claiming that the displayed event proves causation.

A comparative mass laboratory should first show the existing simplified model for a small, explicit set of masses. Add published MIST/PARSEC or other tracks only after verifying dataset version, variables, units, source, citations, coverage and use terms. Never silently merge reference tracks with the current analytic scaling relations.

**Tests:** scenario configurations reproduce from seed; no state leaks between scenarios; camera frames all relevant subjects; mass tracks stay in supported domains; uncertainty labels are visible; default six-stage timeline remains unchanged.

### 19.10 Workstream I — performance, polish and delivery

After each major workstream:

1. Build Release and run unit/renderer self-tests.
2. Run representative benchmark stages at 1920×1080.
3. Record frame-time distribution and GPU time per major pass.
4. Check resident VRAM with all selected assets and targets loaded.
5. Test Performance, Balanced, High and Capture quality tiers.
6. Capture screenshots at identical timestamps and camera positions.
7. Review visual defects before adding another effect.
8. Update only the relevant existing documentation and development log.

Preserve the last known good build. Do not combine several high-risk features into one unreviewed change.

### 19.11 Dependencies and recommended order

| Capability | Depends on | Do not start before |
|---|---|---|
| Cloud filaments/cores | Existing density field and deterministic seed | Baseline volume is stable |
| Adaptive volume sampling | Existing volume pass and GPU profiling | Density morphology is understood |
| Stellar atmosphere layers | Existing star shader/HDR pipeline | Current photosphere is visually stable |
| Particle species | Current deterministic particle IDs and timeline | Baseline paths are tested |
| Magnetic field lines | Explicit vector-field definition | Scientific model and coordinate conventions are documented |
| Supernova instability morphology | Existing pathway-specific ejecta/remnant | Existing NS/BH/WD pathway guards pass |
| Scientific cross-sections | Shared field sampling function | Field units/status are defined |
| Observation mode | Curated, rights-reviewed asset set | Provenance metadata is complete |
| Binary/cluster scenarios | Reusable scene object/camera configuration | Single-star path remains regression-free |
| Comparative tracks | Existing StellarModel and verified reference data | Equations/data provenance are clearly separated |

### 19.12 Expansion milestone completion report

For each workstream report:

- files actually changed;
- feature state (implemented/partial/deferred/rejected);
- tests run and exact results;
- screenshots/capture identifiers;
- measured GPU time and VRAM impact;
- regressions found/fixed;
- scientific assumptions added or changed;
- known limitations remaining;
- next smallest verified milestone.

Do not report estimates as measurements or screenshots as proof of physical validity.
