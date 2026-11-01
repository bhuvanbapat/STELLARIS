# STELLARIS — Project Instructions for Claude Code

## 1. Project Identity

**Project name:** STELLARIS
**Project type:** Computer Graphics academic project
**Subject:** Stellar formation and evolution, from a molecular cloud to a stellar remnant
**Primary objective:** Build a visually exceptional, scientifically defensible, interactive 3D visualization of stellar formation and evolution.

The application must demonstrate meaningful computer graphics techniques, a coherent scientific narrative, interactive controls, and polished visual presentation.

This is not intended to be a commercial game or a full-scale astrophysics research simulator.

The final application must be a real, functioning, locally runnable program—not a mock-up, static presentation, collection of screenshots, or nonfunctional prototype.

## 2. Absolute Rule: Start From a Clean Slate

This is a new implementation.

1. Treat the current project directory as the sole workspace for the new implementation.
2. Do not search for, import, copy, or continue a previous implementation unless explicitly authorized.
3. Do not assume that previous architecture, source code, design decisions, or documentation should be reused.
4. Do not recreate an old implementation merely because its approach is familiar.
5. Do not invent requirements based on inaccessible conversations, remembered project files, or assumptions about earlier attempts.
6. Do not introduce files, frameworks, dependencies, or features without a clear project-related reason.
7. Preserve the separation between the new project and any archived or unrelated directories.
8. Never delete user files or execute destructive commands without explicit authorization.

The new project's specifications must determine its implementation.

## 3. Mandatory Context-Loading Protocol

Before making architectural decisions or writing application code, inspect the current project directory and read the available authoritative project documents.

Load the documents in this order:

### First: Global Instructions

* `CLAUDE.md`

Read this file completely and follow its instructions throughout the session.

### Second: Project Requirements

* `PROJECT_SPEC.md`

This document defines the required functionality, scientific scope, user experience, constraints, and acceptance criteria.

Do not begin implementation until this document exists and has been reviewed.

### Third: Visual Direction

* `VISUAL_DESIGN.md`

This document defines the intended visual identity, rendering quality, composition, lighting, colors, camera behavior, animation, transitions, and interface design.

Use it to guide the actual application rather than producing generic space-themed graphics.

### Fourth: Scientific Model

* `SCIENTIFIC_MODEL.md`

This document defines the scientific concepts being represented, the approximations permitted, the mass-dependent evolutionary pathways, and the distinction between calculated quantities and artistic effects.

Never present an approximation as an exact physical simulation.

### Fifth: Implementation Plan

* `IMPLEMENTATION_PLAN.md`

This document defines the implementation sequence, dependencies, milestones, and verification requirements.

Follow the sequence rather than attempting the entire application in one unstructured operation.

### Loading Rules

1. If a referenced document does not exist, report that fact. Do not pretend to have read it.
2. Do not create substitute requirements from guesses.
3. Do not silently resolve contradictions between documents. Identify the conflicting statements and request clarification when the conflict materially affects implementation.
4. Do not start major implementation while essential requirements remain unresolved.
5. After loading the documents, provide a concise summary of the project, constraints, architecture decisions still pending, and the first implementation milestone.
6. Re-read the relevant sections when beginning a milestone that depends on them.
7. If additional authoritative documents are introduced later, incorporate them into the loading protocol explicitly.

## 4. Source-of-Truth Hierarchy

Use the following hierarchy when determining what to implement:

1. Explicit decisions and corrections provided by the user.
2. `PROJECT_SPEC.md` for functional requirements and scope.
3. `VISUAL_DESIGN.md` for visual and interaction requirements.
4. `SCIENTIFIC_MODEL.md` for scientific assumptions and physical interpretation.
5. `IMPLEMENTATION_PLAN.md` for implementation order and verification.
6. `CLAUDE.md` for operating rules and development discipline.
7. Official technical documentation for framework and library behavior.
8. Your own engineering judgment for implementation details not otherwise specified.

If two authoritative requirements conflict, do not silently choose whichever is easier to implement.

Explain the conflict, its consequences, and the available options before making a consequential decision.

## 5. Architecture Must Be Decided Before Implementation

Do not select the entire technology stack merely because it is familiar or easy to generate.

Evaluate the following considerations:

* Academic Computer Graphics requirements.
* Whether GLUT/freeGLUT is mandatory and whether it must be used in the main application.
* Compatibility with the user's Windows environment and NVIDIA RTX 4050 laptop GPU.
* Rendering capabilities required for volumetric clouds, particles, procedural geometry, shaders, lighting, and post-processing.
* Animation and timeline requirements.
* Development complexity and maintainability.
* Build reproducibility and dependency management.
* Performance and visual-quality requirements.

If faculty requirements concerning GLUT are not yet confirmed, explicitly flag that decision as unresolved.

Do not build the application around an unverified assumption about academic requirements.

Before implementing the main application, document the selected architecture and explain why it satisfies the project requirements.

Prefer the simplest architecture capable of achieving the required visual quality and functionality.

## 6. Product Direction

STELLARIS must prioritize the following:

1. A compelling, cinematic 3D presentation.
2. A coherent progression through stellar formation and evolution.
3. Continuous, deliberate transitions between stages.
4. Meaningful user interaction.
5. A deterministic, controllable animation timeline.
6. Scientifically defensible explanations.
7. Stable execution and reproducible builds.
8. A polished interface that supports the visualization without overwhelming it.

Avoid generic visualizers, disconnected demonstration scenes, excessive interface controls, unnecessary game mechanics, and superficial effects that do not contribute to the scientific narrative.

Visual quality must come from deliberate composition, lighting, depth, scale, motion, rendering techniques, and carefully designed transitions.

## 7. Core Scientific Narrative

The intended narrative consists of six stages:

1. Molecular cloud.
2. Gravitational collapse.
3. Protostar formation.
4. Accretion disc and bipolar jets.
5. Main-sequence star.
6. Mass-dependent stellar evolution and final remnant.

The final stage must represent different evolutionary outcomes according to initial stellar mass.

The precise scientific model, transition behavior, and permitted approximations must be established in `SCIENTIFIC_MODEL.md` and `PROJECT_SPEC.md`.

Do not assume every star follows the same evolutionary pathway.

Do not invent numerical results, physical measurements, or validation claims.

## 8. Scope Control

The project is a Computer Graphics visualization, not an attempt to reproduce every physical process occurring inside a real star.

Do not implement the following unless the authoritative specifications explicitly justify them:

* A complete astrophysical simulation engine.
* A full smoothed-particle hydrodynamics solver.
* Unnecessary gravitational N-body simulation.
* Complex neighbour-search and smoothing-length systems.
* Artificial-viscosity machinery unrelated to the accepted model.
* Detailed stellar-interior or nuclear-reaction simulations.
* Large collections of redundant sliders, graphs, and configuration panels.
* Game mechanics that distract from the scientific visualization.
* Frameworks, services, or infrastructure unrelated to the deliverable.

Prefer a limited, well-explained model and a strong visualization over an enormous, unreliable implementation.

Do not reduce the project to a static animation merely to simplify development. The final scope must retain meaningful interactivity.

## 9. Animation and Timeline Integrity

The timeline must support the interaction requirements defined in `PROJECT_SPEC.md`.

Where playback, pausing, seeking, rewinding, or stage selection is required:

* Animation state must be deterministic for a given timeline position and configuration.
* Seeking must reproduce the appropriate visual state without requiring the user to replay the entire simulation.
* Transitions must remain coherent when the user seeks across stage boundaries.
* Playback speed must be controlled predictably.
* Resetting must return the application to a defined initial state.
* Simulation state and presentation state must not become arbitrarily inconsistent.

Prefer time-indexed animation, deterministic procedural state evaluation, or stored/interpolated states where appropriate.

Do not assume that reversing a numerical simulation automatically produces a correct reverse-time state.

## 10. Engineering Standards

All implementation work must meet these requirements:

* Maintain a clear separation between rendering, application state, animation, scientific calculations, input handling, and interface code.
* Use meaningful names and understandable abstractions.
* Avoid unnecessary complexity and duplicated logic.
* Handle errors explicitly.
* Manage GPU resources and other allocated resources correctly.
* Avoid hard-coded absolute paths.
* Keep configuration and assets organized.
* Avoid adding dependencies without justification.
* Maintain build instructions and document important architectural decisions.
* Never claim a feature works without sufficient evidence.
* Never claim that the application has been built or executed if it has not.

Prefer maintainable code over unnecessarily elaborate abstractions.

Do not sacrifice architecture, correctness, or visual quality to maximize the amount of generated code.

## 11. Mandatory Milestone Workflow

Work incrementally.

For every milestone:

1. Identify the requirements being implemented.
2. Inspect the relevant existing files.
3. State the intended changes and their purpose.
4. Implement only the current milestone's scope.
5. Build the application using the actual configured toolchain.
6. Run relevant automated tests.
7. Execute the application when the environment permits.
8. Inspect actual rendering output when visual changes are involved.
9. Correct discovered failures before declaring the milestone complete.
10. Summarize completed work, verification evidence, known defects, and remaining work.

Do not proceed merely because source code has been generated.

A successful compilation does not establish visual correctness, scientific correctness, or correct interaction behavior.

If execution or visual inspection is unavailable, disclose the limitation explicitly.

Do not fabricate screenshots, test results, performance measurements, or verification evidence.

## 12. Visual Quality Assurance

For rendering-related milestones, verify the actual application output.

Evaluate:

* Overall composition.
* Object scale and depth.
* Lighting and contrast.
* Particle density and distribution.
* Cloud appearance and volume.
* Shader artifacts.
* Temporal coherence.
* Transition quality.
* Camera movement.
* Readability of labels and interface elements.
* Frame-rate stability.
* GPU and memory usage where measurable.

Do not treat placeholder geometry, flat-colored spheres, or generic particle clouds as finished visual assets when the specification calls for a polished cinematic scene.

Temporary placeholders are acceptable during early milestones but must be identified and replaced when required.

## 13. Use of Installed Plugins and Tools

The following Claude Code plugins are installed and enabled for the user:

* `context7`
* `feature-dev`
* `pr-review-toolkit`
* `code-simplifier`

Use these capabilities when they materially improve the work.

### Context7

Use current library documentation when framework APIs, version-specific behavior, or unfamiliar dependencies need verification.

Do not invent APIs or rely on remembered documentation when current documentation is available and relevant.

### Feature Development

Use structured exploration, planning, implementation, and review for substantial features.

Inspect the existing architecture before changing it.

### Code Review

Review significant changes for correctness, edge cases, error handling, resource management, and test coverage.

### Code Simplification

Simplify code after its behavior is established and tested.

Do not perform aggressive refactoring that changes behavior without verification.

Plugin availability does not prove that the resulting code is correct.

Use actual builds, tests, and visual inspection as the evidence of correctness.

Do not install additional plugins, tools, or dependencies automatically unless they are justified by the project requirements.

## 14. User Skill Level and Communication

Assume the user is a beginner in graphics programming and relies heavily on Claude Code for implementation.

Therefore:

* Handle routine implementation details autonomously.
* Explain consequential technical decisions in clear language.
* Avoid requiring the user to write code manually.
* Provide exact commands when manual action is necessary.
* Explain where commands must be entered.
* Do not overwhelm the user with unnecessary implementation details.
* Do not hide important risks or unresolved decisions.
* Do not claim certainty when the evidence is insufficient.
* Ask focused questions when a decision materially affects architecture, academic compliance, cost, or project scope.

Autonomy means completing well-defined work without unnecessary interruptions. It does not mean making irreversible decisions or concealing uncertainty.

## 15. Git and Project Safety

Use version control when available and appropriate.

Before substantial implementation milestones:

* Check the current working-tree status.
* Preserve meaningful checkpoints.
* Avoid overwriting unrelated work.
* Review changes before committing.
* Keep generated build artifacts out of source control where appropriate.

Never delete the project, reset the repository, discard user changes, or overwrite unrelated files without explicit authorization.

Do not assume a repository is disposable merely because it is a new project.

## 16. Completion Criteria

The project is not complete merely because the code exists.

Completion requires evidence that:

* The application builds successfully using the documented environment.
* The application launches and runs.
* All required stages are implemented.
* Required interactions work correctly.
* Timeline seeking and transitions behave deterministically.
* The scientific presentation follows the approved model.
* The visual output satisfies the approved design direction.
* Performance is acceptable on the target hardware.
* Known limitations are documented honestly.
* Setup, build, and execution instructions are available.
* The final application has been tested against the acceptance criteria in `PROJECT_SPEC.md`.

The acceptance criteria in the authoritative project specification determine the exact required deliverables.

## 17. First Action in a New Session

Before writing application code:

1. Read this file completely.
2. Inventory the current project directory.
3. Identify which authoritative project documents exist.
4. Read all available authoritative documents in the prescribed order.
5. Report missing documents and unresolved decisions.
6. Confirm the project scope and constraints.
7. Determine which architectural decisions must be settled before implementation.
8. Propose the first verifiable milestone.

Do not generate the entire application immediately.

Do not invent missing specifications.

Do not begin major implementation until the essential requirements and architecture have been established.

**The objective is to deliver a working, polished, scientifically defensible STELLARIS application—not merely to generate a large quantity of code.**

## 18. Project State and Tooling (added 2026-11-02)

* Read `docs/DEVELOPMENT_LOG.md` at the start of every session or after context compaction. It records architecture decisions, scientific sources, and milestone status. Append to it at the end of every milestone.
* `docs/CLAUDE_SETUP.md` documents the Claude Code configuration for this project: project skills in `.claude/skills/` and the Stop verification hook in `.claude/settings.json`.
