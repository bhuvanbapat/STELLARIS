# STELLARIS — Visual Design Specification

## 1. Design Mission

STELLARIS must feel like an immersive journey through the birth and evolution of a star.

The visual experience should combine the depth of a scientific visualization, the atmosphere of a cinematic space sequence, and the clarity of a carefully designed interactive exhibit.

The viewer should feel that they are observing a vast physical process unfolding across enormous scales of space and time.

The application must not resemble:

* A generic space screensaver.
* A basic OpenGL laboratory assignment.
* A flat infographic with a 3D object behind it.
* A conventional game interface covered in unnecessary HUD elements.
* A collection of unrelated particle effects.
* A glowing sphere surrounded by randomly moving dots.

**Every major visual element must contribute to the scientific narrative, spatial composition, or atmosphere.**

Visual quality must be achieved through deliberate design and implemented rendering techniques—not merely through increasing particle counts or adding bloom.

---

## 2. Visual Identity

### 2.1 Overall Character

The visual language should be:

* Cinematic.
* Atmospheric.
* Three-dimensional.
* Scientifically grounded.
* Restrained but visually rich.
* Elegant.
* Immersive.
* Technically polished.

The experience should balance darkness and luminosity, using light to reveal the structure of the stellar environment.

Avoid excessive brightness, indiscriminate glow, oversaturated colors, and visual noise.

### 2.2 Visual Hierarchy

The viewer's attention must follow a deliberate hierarchy:

1. The principal astronomical phenomenon.
2. Its surrounding physical structures.
3. The camera composition and environmental depth.
4. Scientific annotations and stage information.
5. Playback and navigation controls.

The interface must never compete unnecessarily with the main visual subject.

### 2.3 Originality

STELLARIS must develop a coherent visual identity rather than imitate a specific commercial game, film, or existing visualization.

Use original compositions, procedural structures, material treatments, camera movements, and transitions.

References may inform quality and atmosphere, but the implementation must remain appropriate to the academic project's scientific purpose.

---

## 3. Color Language

### 3.1 Environmental Foundation

Use a predominantly dark environment to preserve contrast and establish depth.

The environment may incorporate:

* Near-black space.
* Deep blue-black shadows.
* Restrained navy and indigo atmospheric tones.
* Subtle cool highlights.
* Carefully placed warm emission.

Pure black should not be the only environmental color. Very subtle tonal variation should help distinguish spatial layers.

### 3.2 Molecular Cloud

The cloud should use layered, restrained combinations of:

* Deep blue.
* Blue-violet.
* Muted indigo.
* Dusty magenta.
* Subtle warm highlights where justified by the lighting design.

These colors are artistic choices, not claims that a real molecular cloud necessarily appears exactly this way to the naked eye.

The cloud must remain dimensional and structured rather than becoming a uniform purple fog.

### 3.3 Gravitational Collapse

As the cloud contracts:

* Increase the visual concentration of the central region.
* Introduce controlled warm emission where appropriate.
* Preserve cooler tones in the surrounding material.
* Use contrast to communicate increasing central density.

The transition must feel continuous rather than switching abruptly to an unrelated color scheme.

### 3.4 Protostar and Accretion Structures

The protostar may introduce restrained amber, gold, and warm white.

The surrounding disc should remain distinguishable from the central object.

Jets may use a different visual treatment from the disc, but their luminosity must not obscure the underlying geometry.

### 3.5 Main-Sequence Star

Use a palette appropriate to the selected stellar presentation.

Possible treatments include:

* Warm gold and white.
* Neutral white.
* Blue-white for a suitably represented hotter star.

Do not imply that every star has the same color or surface temperature.

If the visualization represents one illustrative star, its assumed properties must remain consistent with the scientific model.

### 3.6 Final Stellar Outcomes

Use clearly differentiated visual treatments for the final evolutionary outcomes.

The differences must arise from the nature of the represented object and the chosen composition—not arbitrary color changes alone.

For example:

* A white dwarf should communicate a compact, hot remnant.
* A neutron star should communicate an extremely compact remnant.
* A black hole may be represented through a dark central region and surrounding gravitationally influenced or accretion-related visual structures where appropriate.

Do not portray a black hole as a normal luminous sphere.

---

## 4. Three-Dimensional Depth

The scene must communicate volume.

Use an appropriate combination of:

* Layered structures.
* Spatially distributed particles.
* Density variation.
* Perspective.
* Depth-dependent lighting.
* Occlusion.
* Camera movement.
* Atmospheric scattering or an appropriate approximation.
* Distinct foreground, middle-ground, and background elements.

Avoid distributing all visual elements uniformly across a flat screen-facing plane.

Particles should have a meaningful spatial distribution and should not simply form an evenly spaced spherical shell.

The viewer must be able to distinguish the main subject from the surrounding environment.

Depth techniques should be selected according to the chosen rendering architecture and available performance budget.

---

## 5. Molecular Cloud Rendering

The molecular cloud is the first major visual impression and must receive substantial attention during development.

### 5.1 Shape

The cloud should have an irregular, evolving structure.

Avoid:

* A perfect sphere of particles.
* A single translucent sphere.
* A uniform fog volume.
* An evenly distributed shell.
* Random noise without coherent large-scale structure.

Use a combination of large-scale shape variation and smaller-scale detail.

The overall silhouette must remain visually readable.

### 5.2 Density Structure

Create meaningful variation in density.

Potential techniques include:

* Procedural density fields.
* Layered noise.
* GPU particle distributions.
* Volumetric rendering.
* Procedural geometry.
* Multiple spatially separated cloud structures.

These are alternative or complementary techniques to evaluate, not a requirement to implement every technique.

Prefer a coherent density structure over a large quantity of visually meaningless particles.

### 5.3 Lighting

Lighting should reveal cloud structure without washing out the volume.

Use controlled illumination and contrast.

Avoid making every particle equally bright.

### 5.4 Motion

Motion should be subtle and purposeful.

The cloud may contain slow internal movement, but the initial stage must not appear to be undergoing rapid collapse before that stage begins.

Motion must remain deterministic where required by the timeline system.

---

## 6. Gravitational Collapse Visual Direction

The collapse sequence must communicate a physical transformation.

Visual changes should include a coherent combination of:

* Contraction of the principal cloud structure.
* Increasing central concentration.
* Changing density distribution.
* Material movement toward the central region.
* Gradual changes in emission.
* Changes in the relative scale of the surrounding environment.

The sequence should preserve recognizable material and spatial continuity across the transition.

Avoid simply shrinking an entire cloud uniformly into a bright sphere.

Do not use camera zoom alone as a substitute for showing the collapse process.

The viewer should be able to distinguish physical movement within the scene from camera movement.

---

## 7. Protostar Rendering

The protostar must emerge as a distinct central object.

Its appearance should combine:

* A defined luminous center.
* Controlled emissive shading.
* Appropriate surrounding material.
* A restrained halo or atmospheric effect.
* A readable relationship to the surrounding cloud and disc.

Avoid using an oversized bloom effect to conceal a poorly rendered central object.

The central object should retain visual definition even when luminous.

The transition from collapse to protostar must not look like an unrelated object being inserted into the scene.

---

## 8. Accretion Disc and Bipolar Jets

### 8.1 Accretion Disc

The disc must read clearly as a rotating structure surrounding the central object.

It should have:

* A distinguishable central region.
* A coherent radial structure.
* Meaningful spatial thickness or layering where appropriate.
* A visually legible orientation.
* Controlled lighting and emission.
* Motion consistent with the simplified presentation.

Avoid representing the disc as a perfectly flat, featureless ring.

### 8.2 Bipolar Jets

The jets should extend along a coherent axis from the central region.

They should be visibly distinct from the disc.

Potential visual techniques include:

* Particle streams.
* Procedural tapered structures.
* Animated density patterns.
* Shader-based emission.
* Controlled turbulence.

The selected technique must support the required visual quality and performance.

Avoid uniformly thick cylinders with no structure unless used as temporary placeholders.

### 8.3 Composition

The camera must reveal the relationship between the central object, disc, and jets.

If the camera moves, the composition must continue to communicate the geometry.

Do not allow the disc or jets to disappear behind the central object for extended periods without a deliberate compositional reason.

---

## 9. Main-Sequence Star Rendering

The main-sequence star should feel visually distinct from the protostar.

The design may incorporate:

* A luminous surface.
* Controlled surface variation.
* Subtle animated patterns.
* A restrained corona.
* Atmospheric emission.
* Carefully controlled bloom.
* Appropriate color temperature.

Do not make the star appear to be a smooth plastic sphere with a bright texture.

Do not use surface effects that imply scientific accuracy beyond what is actually modeled.

The star's surface animation must remain stable during seeking and playback.

A complex shader is not inherently better than a simpler shader that produces a coherent and convincing result.

---

## 10. Stellar Evolution and Remnant Presentation

The final stage should feel like a meaningful conclusion to the preceding sequence.

The presentation must distinguish the evolutionary pathways without making them appear to be identical objects with different colors.

Possible visual elements include:

* Expansion and ejection of outer material.
* A supernova-like event where scientifically appropriate to the chosen pathway.
* A compact white dwarf.
* A neutron star represented at an appropriate illustrative scale.
* A black hole with a carefully designed surrounding environment where appropriate.

Do not depict a supernova in every stellar evolutionary pathway.

Do not imply that a compact remnant is visually comparable in physical size to the original star unless the visualization clearly identifies its sizes as symbolic.

If objects are enlarged or scales are compressed for visibility, communicate that limitation where scientifically relevant.

The final stage must prioritize conceptual clarity over spectacle that misrepresents the process.

---

## 11. Camera Direction

Camera movement is a core component of the cinematic experience.

### 11.1 Initial Composition

The initial view must establish:

* The scale of the cloud.
* The principal region of interest.
* The surrounding spatial environment.
* A clear visual path for the viewer's attention.

The main subject must not be clipped or poorly positioned.

### 11.2 Camera Movement

Use controlled movement where it improves the presentation.

Possible movements include:

* Slow orbital movement.
* Deliberate dolly movement.
* Controlled push-ins.
* Gradual reframing.
* Carefully timed transitions between viewing distances.

Avoid:

* Unmotivated rapid spinning.
* Constant camera shake.
* Excessive zooming.
* Sudden jumps between unrelated viewpoints.
* Movement that makes the scientific process difficult to understand.

### 11.3 Camera and Timeline

Camera animation must be coordinated with the timeline.

Seeking backward or forward must reproduce the appropriate camera state.

Manual camera interaction, if provided, must not permanently corrupt the authored cinematic path.

The initial camera state must always be recoverable through reset.

---

## 12. Cinematic Timeline and Transitions

The application should feel like one continuous experience rather than six unrelated demonstrations.

### 12.1 Stage Transitions

Transitions should preserve continuity where possible.

Use appropriate combinations of:

* Geometry changes.
* Density-field changes.
* Particle movement.
* Lighting changes.
* Camera movement.
* Controlled changes in emission.
* Interpolation between visual states.
* Temporary transition effects.

Avoid abrupt scene replacement when a continuous transformation is possible and meaningful.

### 12.2 Transition Timing

Transition duration must support comprehension.

Do not accelerate every event merely to make the experience feel dramatic.

Allow the viewer to observe important structures before moving to the next stage.

### 12.3 Timeline Seeking

Every transition must work when the viewer seeks directly into or out of it.

The animation must not depend on an event having previously occurred during forward playback.

Seeking must produce the correct state for the selected timeline position.

### 12.4 Visual Continuity

Continuity does not require every object to remain physically present throughout every stage.

However, when the scene changes, the transition must be deliberate and understandable.

---

## 13. Interface Visual Design

The interface must support the cinematic presentation rather than dominate it.

### 13.1 General Appearance

Use a restrained, modern scientific interface.

The interface should feel integrated with the scene.

Avoid excessive borders, bright panels, large opaque windows, decorative elements with no purpose, and unnecessary controls.

### 13.2 Stage Identification

Display the current stage clearly.

Provide concise scientific context that can be read during the demonstration.

Use consistent hierarchy and spacing.

### 13.3 Timeline

The timeline must clearly communicate:

* Current position.
* Stage boundaries.
* Stage labels.
* Playback state.
* The ability to seek.

It should remain visually subordinate to the main scene.

### 13.4 Controls

Use clear controls for:

* Play and pause.
* Reset.
* Timeline navigation.
* Playback speed.
* Required stage selection.

Additional controls must be justified by the approved functional specification.

### 13.5 Readability

Typography must remain readable against the background.

Avoid placing essential text over highly detailed or bright regions.

Provide enough contrast without making the interface visually harsh.

---

## 14. Lighting and Post-Processing

Lighting and post-processing should enhance the scene rather than conceal its weaknesses.

Evaluate appropriate use of:

* Emissive materials.
* HDR rendering where supported.
* Tone mapping.
* Bloom.
* Depth-aware atmospheric effects.
* Contrast control.
* Color grading.
* Soft transitions between illuminated and shadowed regions.

The final architecture must determine which effects are practical and worthwhile.

### Constraints

* Bloom must not erase object boundaries.
* Bright highlights must retain useful detail.
* Dark regions must not collapse into featureless black.
* Effects must not introduce distracting halos.
* Post-processing must not make the application unresponsive.
* Every effect must contribute to the final composition.

A restrained result with clear structure is preferable to excessive glow.

---

## 15. Motion and Particle Quality

All particle-based effects must have a deliberate purpose.

Particle systems should be evaluated for:

* Spatial distribution.
* Scale variation.
* Motion coherence.
* Density.
* Depth.
* Lighting.
* Temporal stability.
* Rendering cost.

Avoid:

* Uniform random distributions.
* Excessive particle counts used to hide weak geometry.
* Unmotivated movement.
* Flickering.
* Sudden changes in particle identity during seeking.
* Motion that contradicts the intended physical process.

Where deterministic seeking is required, particle positions should be reproducible from timeline position and configuration, or be derived from a deterministic state model.

---

## 16. Performance-Aware Art Direction

Visual ambition must remain compatible with the target hardware.

The application must prioritize the visual techniques that produce the greatest improvement in perceived quality.

Do not assume that every effect requires maximum resolution or maximum particle count.

Evaluate:

* GPU cost.
* VRAM use.
* Overdraw.
* Shader complexity.
* Volume-rendering cost.
* Particle update cost.
* Frame-time stability.
* Window resolution.

Use scalable quality settings only where they materially improve reliability or allow useful visual trade-offs.

The initial implementation should establish a measured performance baseline before expensive effects are added.

---

## 17. Visual Development Workflow

Develop the visual experience incrementally.

### Pass 1 — Composition

Establish the camera, scale, scene layout, and visual hierarchy.

### Pass 2 — Major Structures

Build the main cloud, central object, disc, jets, and relevant environmental structures.

### Pass 3 — Materials and Lighting

Establish surface appearance, emission, illumination, contrast, and depth.

### Pass 4 — Motion

Add controlled animation and transitions.

### Pass 5 — Atmosphere

Add carefully selected particles, volumetric effects, and post-processing.

### Pass 6 — Interface

Integrate the timeline, controls, stage labels, and scientific annotations.

### Pass 7 — Refinement

Inspect actual rendered output, identify the weakest visual elements, and improve them.

### Pass 8 — Verification

Test stage transitions, seeking, playback, resizing, performance, and visual stability.

Do not treat an early gray-box scene as the finished visual result.

---

## 18. Visual Acceptance Criteria

The visual experience must pass the following checks:

* [ ] The initial molecular cloud has a distinctive and readable silhouette.
* [ ] The cloud communicates three-dimensional structure.
* [ ] The scene has clear foreground, middle-ground, and background separation where appropriate.
* [ ] Gravitational collapse is visually distinguishable from camera zoom.
* [ ] The protostar has a defined luminous center.
* [ ] The accretion disc and bipolar jets are clearly distinguishable.
* [ ] The main-sequence star has a distinct appearance.
* [ ] Final stellar outcomes communicate their differences clearly.
* [ ] Lighting enhances structure without excessive bloom.
* [ ] Camera movement supports comprehension.
* [ ] Transitions are deliberate and temporally coherent.
* [ ] Seeking reproduces the intended visual state.
* [ ] The interface remains readable without dominating the scene.
* [ ] Major rendering artifacts have been inspected.
* [ ] The result does not depend on excessive particle counts to appear detailed.
* [ ] Performance has been measured on the target hardware.
* [ ] The final visual quality has been assessed from actual application output.

Do not mark a criterion complete without examining sufficient evidence.

---

## 19. Final Visual Standard

STELLARIS should make the viewer want to continue exploring.

The scene must have depth, the lighting must have purpose, the motion must communicate change, and the transitions must support the scientific narrative.

The visual experience should feel like a coherent journey through stellar formation and evolution rather than a sequence of unrelated effects.

**The standard is cinematic clarity supported by credible scientific visualization—not spectacle at the expense of understanding.**
