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

---

# 20. Expanded Visual Systems and Art Direction

**Status:** This section defines the intended next visual-development direction. It does not certify that the systems are present in the current executable. The existing visual identity, palette, camera language, timeline composition, and acceptance rules remain in force.

## 20.1 The governing visual principle

STELLARIS should feel like a coherent astronomical environment whose structure becomes more legible as the viewer moves closer. Do not turn it into a generic space game, a screensaver, or a collection of unrelated VFX demonstrations.

Every new visual element must answer at least one question:

- What physical structure or phenomenon does it help the viewer recognise?
- What spatial scale does it communicate?
- What transition in the stellar narrative does it clarify?
- What scientific quantity or limitation does it make inspectable?

If an effect answers none of these, remove it or keep it as a disabled debug experiment.

## 20.2 Environmental composition: from a cloud to a structured region

The molecular-cloud environment should be composed in layers with a clear hierarchy:

1. **Regional envelope:** a broad, irregular volume that establishes the scene silhouette and depth.
2. **Filament network:** elongated ridges and connected strands that guide the eye through the cloud rather than filling every pixel with noise.
3. **Dense condensations:** one dominant core and optional secondary cores, readable at medium distance.
4. **Globules and dust structures:** dark, compact silhouettes and lanes that interrupt the illuminated volume.
5. **Cavities and low-density gaps:** negative space that reveals depth and prevents a uniformly filled volume.
6. **External illumination:** directional illumination or a nearby stellar source that reveals volume boundaries without washing out the entire cloud.
7. **Small-scale detail:** wisps and clumping, visible mainly in close inspection and suppressed when below pixel scale.

Use a strong foreground/midground/background separation. Avoid placing bright, equally detailed filaments at every depth. The viewer should be able to identify the main cloud mass, core, and direction of collapse within a few seconds of viewing.

### 20.2.1 Filament design

Filaments should have elongated, curved geometry or density ridges with variable thickness and branching. Their direction and contrast should be composed around the focal core. Avoid evenly spaced parallel tubes, repeated spirals, perfect radial spokes, and uniform thickness. A filament should emerge from the density field, not look like a neon cable laid over a fog sphere.

### 20.2.2 Dense-core design

The core must be visible as a genuine density concentration: darker/stronger extinction where appropriate, a more compact silhouette, and a clear transition into surrounding material. Do not simply add a bright sphere at the centre. If the core is rendered as an emissive or illuminated feature for readability, its artistic treatment must not imply a hot luminous star before the protostar stage.

### 20.2.3 Globules and dust lanes

Use compact high-opacity regions and irregular low-frequency boundaries. Their purpose is to create depth, contrast, and visual cues of obscuration. They should not resemble small planets or hard opaque rocks.

## 20.3 Volumetric-quality targets

The cloud should feel volumetric because light is attenuated and scattered through depth, not because bloom makes it glow.

Required visual properties:

- readable depth layering and soft internal occlusion;
- stable density features with no visible temporal swimming;
- anisotropic structure without obvious repeated noise tiles;
- controlled bright edges and illuminated interiors;
- smooth transitions between dense and diffuse material;
- no uniform bright fog sheet around the centre;
- no visible ray-step bands at normal viewing distance;
- no low-resolution upsample halo around the star or opaque geometry;
- no sudden change in brightness when the viewer seeks to a timestamp.

Use the current scattering/absorption implementation as the baseline. Improvements should target the visible failure mode (for example, banding, unstable shadows, weak depth, or overbright density) rather than adding several lighting terms simultaneously. Compare before/after captures at identical camera, time, exposure and quality.

### 20.3.1 Multiple-scattering appearance

If an ambient or multiple-scattering approximation is added, it should soften deep shadow regions and add plausible low-level energy without making the entire cloud self-luminous. Do not use an unconstrained ambient term to hide missing shadow detail. The cloud must retain dark regions and a visible density hierarchy.

### 20.3.2 Adaptive sampling appearance

Adaptive ray sampling must not create a change in texture frequency, hard boundaries, flickering detail, or silhouette discontinuities. Sample density must remain high enough around thin filaments and core edges. The low-quality preset may simplify detail but should not create obvious geometric banding.

## 20.4 Stellar body: layered emission, not a glowing ball

The star should have a clear visual hierarchy from surface to outer atmosphere.

### 20.4.1 Photosphere

- Use an emissive surface with temperature-informed chromaticity.
- Preserve the existing limb-darkening treatment and make its coefficient/model explicit.
- Granulation-like details should be low-contrast, multi-scale and temporally coherent.
- Optional starspots should be cooler/darker regions with organic boundaries and stable positions.
- Do not use a high-contrast cellular noise pattern that resembles cracked lava or a procedural marble texture.
- Surface detail should scale with camera distance; sub-pixel detail must fade rather than alias.

### 20.4.2 Chromosphere

The chromosphere should be a thin, deliberately composed layer visible at the limb and in selected close-up views. It should use a distinct emission/opacity response from the photosphere. Do not create a thick, uniformly opaque shell that changes the perceived stellar diameter unpredictably.

### 20.4.3 Corona

The corona should extend beyond the photosphere with asymmetric structures and a low-density appearance. Avoid a perfect spherical glow. Use subtle streamers, local bright regions, and controlled radial falloff. Bloom is a finishing response, not the corona itself.

### 20.4.4 Prominences and loops

Prominences should appear as curved, anchored structures with controlled thickness and emission. Use a small number of clearly visible structures rather than hundreds of tangled lines. Their silhouette should remain legible against both dark space and the bright star. The underlying curves are visual/analytic unless physically generated by a validated field model.

### 20.4.5 Flares and ejections

A flare should have a localised source, a short defined life cycle, and a controlled brightness peak. A CME-like event should expand away from that region with an identifiable direction and shell/particle structure. Avoid a full-frame white flash or a generic explosion sprite. These are deterministic prescribed visual events unless a physical model is introduced.

## 20.5 Particle phenomena: visually distinct species

The particle system should communicate different material motions. Avoid using the same point-sprite appearance for every effect.

| Species | Visual treatment | Avoid |
|---|---|---|
| Infall | subdued, convergent, increasingly concentrated near the core | random dots that merely disappear at the centre |
| Disc material | thin orbital distribution with radial/differential structure | uniform flat rings and identical angular speed |
| Jet knots | collimated, axial, brighter knots within a softer outflow | two perfect laser beams |
| Stellar wind | sparse outward flow with directional bias | spherical confetti |
| CME/ejecta | localised, shell-like outward motion | an expanding billboard detached from the surface |
| Planetary-nebula material | layered, asymmetric shell with a coherent central source | a perfect glowing torus unless intentionally selected |
| Supernova ejecta | irregular, stratified, clumpy expansion | a single smooth sphere of uniform particles |
| Ambient dust | sparse, low-contrast depth cues | high-brightness foreground particles that obscure the subject |

Particle size, brightness and density must respond to view distance and resolution. Use screen-space coverage/fading to prevent sub-pixel flicker. Avoid additive blending for every species; dense additive particles will wash out structure. Choose blend mode based on whether the particle represents emission, opacity, or a tracer overlay.

## 20.6 Magnetic-field visual language

Field lines should be thin, controlled and subordinate to the object. Use a restrained colour/brightness distinct from emitted plasma. Their purpose is explanatory: show orientation, topology and connection to selected structures. Avoid thick neon tubes, dense tangles, arbitrary loops, or field lines that appear to pass through opaque geometry without depth handling.

When a field-line overlay is active, show a compact label such as `ANALYTIC DIPOLE FIELD — VISUALIZATION` or the accurate model name. The overlay must be easy to disable for cinematic shots.

## 20.7 Supernova and remnant art direction

The event should read as a sequence, not a flash followed by a growing ball:

1. **Onset:** focused core/central brightening and a clear change in the central object.
2. **Shock propagation:** a distinct expanding front, not merely a glow radius.
3. **Ejecta separation:** inner/outer layers with non-uniform brightness and density.
4. **Instability-like morphology:** selected fingers, knots and filaments that break spherical symmetry.
5. **Remnant:** persistent asymmetric shell, clumps and fading internal emission.

Irregularity must have a coherent spatial scale and direction. Do not use independent high-frequency noise at every frame; it will shimmer. The same scenario seed/time must reproduce the same structures. Label instability-inspired morphology as a procedural visual representation, not a hydrodynamic calculation.

## 20.8 Formation scenarios without visual duplication

Optional scenarios must look distinct because their initial conditions and structures differ, not because each receives a different arbitrary palette.

- **Low-/intermediate-mass:** a compact core and relatively restrained environment, followed by the existing white-dwarf-compatible route.
- **Massive-star:** a more massive core, stronger environmental feedback cues, and the appropriate high-mass branch, with uncertainty stated.
- **Binary:** two spatially separated condensations/protostars, a shared environment, and readable circumbinary structure. The camera must frame both objects; do not force the single-object close-up path on this scenario.
- **Clustered:** a limited number of cores at distinct positions/stages, with a clear primary subject and restrained background members. Avoid hundreds of decorative stars that look like a generic starfield.
- **Triggered-compression:** a visible incoming compression front and a compressed ridge/core. The effect must be legible as a sequence; do not imply a causal result beyond the simplified model.

Do not add all scenarios to the main timeline as extra numbered stages. They are optional presets or exploratory scenes that reuse the shared six-stage system where possible.

## 20.9 Scientific inspection mode

The scientific mode should reduce visual spectacle enough to expose structure. It should be a deliberate alternate presentation, not a permanently visible wall of controls.

Possible overlays:

- density/opacity heat map;
- temperature reference or prescribed map;
- velocity vector field where available;
- particle trajectory trails/species;
- analytic magnetic-field lines;
- clipping plane/cross-section through the volume;
- selected object measurements and stage metadata.

Use consistent legend placement, readable units, and a clear colour scale. Do not use rainbow colour maps by default. Choose perceptually ordered maps and state the range. If a value is normalized, include `normalized` in the label. If the display is a prescribed animation field, say so.

## 20.10 Astronomical observation and multi-wavelength presentation

Authentic observations should be displayed as evidence/reference, not decorative wallpaper. A useful layout is a compact side-by-side comparison:

- left: authentic image and its object, instrument, filter/band, source and credit;
- right: corresponding STELLARIS procedural scene;
- caption: `conceptual visualization — not a 3D reconstruction` unless a validated reconstruction has been implemented.

For multi-wavelength mode, make each band identifiable and use actual corresponding observation products where available. Infrared, ultraviolet, optical, X-ray and radio imagery can encode different emission/absorption processes and may be false-colour composites. Do not recolour one optical image to imitate these bands.

### 20.10.1 Image treatment

- Preserve aspect ratio and original framing unless a documented crop is necessary.
- Record any crop, colour adjustment, channel mapping, denoising, sharpen, resampling, or compression.
- Do not silently remove source labels or credits embedded in the image.
- Avoid mapping a 2D image onto a spherical cloud surface as if it were measured 3D texture.
- Avoid stretching a small image to fill a 4K display; use its native resolution and a suitable filter.
- Clearly identify composite or processed colour.

### 20.10.2 Resolution and visual cost

Use high-resolution assets only when their projected screen coverage justifies them. The 6 GB VRAM target requires a managed texture budget. Include mipmaps and all derived GPU copies in the estimate. Prefer a well-filtered 2K texture over a poorly sourced or heavily compressed 4K image. Use tiled/streamed or compressed assets only when the pipeline and licensing permit them.

## 20.11 Multi-object camera and composition

New scenarios require camera compositions that respect the object count and scene scale. For a binary, keep both stars and their shared disc visible during overview shots; for a cluster, establish the whole region before moving to a selected core. Keep the existing authored Catmull–Rom camera path and user offset model unless a genuine multi-target requirement requires an extension.

Camera transitions should be continuous in position, orientation and apparent scale. Avoid interpolating raw Euler angles across wrap boundaries. Use a stable look direction/target and ensure near/far planes are appropriate for large cloud extents and close stellar shots.

## 20.12 Visual quality presets

The visual settings must expose a small set of meaningful presets rather than dozens of sliders:

- **Performance:** lower volume resolution/step count, reduced particle density and simplified optional overlays.
- **Balanced:** default real-time balance.
- **High:** higher volume/particle detail where the hardware supports it.
- **Capture:** fixed deterministic settings for screenshots; it may run more slowly.

All presets must preserve the same scene state, model parameters and timeline position. The capture preset must not introduce non-deterministic temporal history.

## 20.13 Visual review protocol for every new system

Capture before/after images with the same time, mass, rotation, camera, exposure and resolution. Review at least:

1. a wide establishing view;
2. a medium subject view;
3. a close detail view;
4. a dark/low-exposure case;
5. the most dense/bright worst-case case;
6. a timeline seek directly into the middle of the effect;
7. a stage boundary immediately before and after activation;
8. the Performance and Capture quality presets.

Inspect for silhouette, scale, depth, aliasing, banding, transparency sorting, flicker, clipping, unintended halos, shader noise repetition, and UI occlusion. A feature is not visually accepted because one screenshot looks attractive.

## 20.14 Expanded visual acceptance checklist

- [ ] Cloud envelope, filaments, dense core, dust and voids read as one coherent environment.
- [ ] Volume retains depth and contrast rather than becoming uniform glow.
- [ ] Adaptive sampling/upsampling creates no visible discontinuity or halos.
- [ ] Photosphere, chromosphere and corona are visually distinguishable.
- [ ] Prominences/flares have a localised and understandable origin.
- [ ] Particle species have distinct motion and blend treatment.
- [ ] Field lines remain explanatory and unobtrusive.
- [ ] Supernova ejecta are asymmetric and structured without looking like random confetti.
- [ ] Binary/cluster cameras preserve the relevant subjects in frame.
- [ ] Scientific overlays are legible, labelled and accurately scaled.
- [ ] Authentic observations have visible provenance/credit and correct band identification.
- [ ] Quality presets change render cost, not scene state.
- [ ] The six-stage default retains its original composition and pacing.
