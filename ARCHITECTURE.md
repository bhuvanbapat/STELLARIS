# STELLARIS — Architecture

## 1. Decision summary

| Topic | Choice | Reason |
|---|---|---|
| Language | C++20 (MSVC 14.44) | already installed; full control over the GPU |
| Graphics API | OpenGL 4.5 core profile | compute shaders, direct state access, debug output; runs on the target RTX 4050; same API family as the GLUT requirement |
| Window / input | GLFW 3.4 | small, reliable, sets per-monitor DPI awareness on Windows |
| GL loader | glad 2.0.8 (generated once, vendored) | no generator needed at build time |
| Math | GLM 1.0.1 | GLSL-like types on the CPU |
| UI | Dear ImGui 1.91.5 | immediate-mode UI drawn in the same GL context; easy custom widgets (timeline) |
| Build | CMake ≥ 3.24 + FetchContent, VS 2022 generator | reproducible: pinned release tags, one command |
| Tests | built-in minimal harness + CTest | no extra dependency |
| GLUT requirement | separate program `glut_demo/` with freeGLUT 3.6.0 | the faculty requires an actual GLUT/OpenGL demonstration, not that the main app use GLUT (confirmed by the user) |

Rejected alternatives:
- Game engines (Unity, Unreal, Godot): large installs, and they hide the
  graphics techniques the course is assessing.
- WebGL / three.js: the specification requires a native desktop application.
- Building the main app on GLUT: GLUT offers no modern context or UI
  facilities and would make the cinematic pipeline (HDR targets, compute,
  ImGui) awkward. GLUT is demonstrated separately and fully instead.

## 2. Source layout

```
app/
  src/sim/      pure C++ (no OpenGL), unit-tested
    Timeline       stage table, playback controller (play/pause/seek/speed/reset)
    StellarModel   mass -> L, R, T_eff, lifetime; blackbody color; pathway categories; free-fall time
    SceneState     evaluateScene(time, config): every visual parameter, pure function
    CameraPath     authored Catmull-Rom camera(time, pathway) + user orbit offset
    StageContent   on-screen scientific text per stage
    CloudField     seeded procedural cloud structure (envelope, filaments, core, globules), baked to a grid
  src/gfx/      OpenGL helpers: RAII handles, Shader (with #include, hot reload), procedural meshes
  src/render/   Renderer (all passes), ParticleSystem (static attributes, deterministic seed)
  src/ui/       Dear ImGui panels, transport controls, timeline, 3D annotations
  src/app/      App: window/context, main loop, input, screenshots, --capture/--selftest/--benchmark
  src/core/     logging, PNG writer
  shaders/      GLSL 4.50 (loaded at runtime; copied next to the exe)
glut_demo/      independent freeGLUT program (own CMakeLists.txt)
tests/          unit tests of the sim layer and particle generation
third_party/glad
cmake/Dependencies.cmake
```

Dependency direction: `app -> ui, render -> gfx -> (GL)`, and every layer may use
`sim`. `sim` depends on nothing but GLM, which is why it can be tested without a GPU.

## 3. Timeline and determinism

The timeline is 148 s at 1× speed: Cloud 0–18, Collapse 18–40, Protostar
40–58, Disc & Jets 58–82, Main Sequence 82–100, Fate 100–148. These are
presentation seconds, not physical time; the UI says so.

The only mutable playback state is `(time, playing, speedIndex)`. Each frame:

```
time      <- time + clamp(realDt, 0, 0.1) * speed   (only while playing)
scene     <- evaluateScene(time, {mass, rotation})   (pure)
camera    <- poseFromOrbit(authoredOrbit(time, pathway), userOffset)
particles <- computed in the vertex shader from (seed, time, scene)
```

Nothing accumulates from frame to frame, so:
- seeking, rewinding and stage selection reproduce exactly the state of that time;
- playback speed changes only the rate, never the state;
- reset sets time 0, paused, 1×, and clears the camera offset;
- frame rate does not affect what is shown at a given time.

Unit tests check all of these properties, as well as continuity across stage
boundaries.

## 4. Frame pipeline (Renderer::render)

1. **Sky**: full-screen procedural star field and galactic dust band → HDR target (RGBA16F).
2. **Opaque bodies**: star / protostar / giant sphere and compact remnant (depth written).
3. **Volume**: ray-marched molecular cloud (structure from the baked `CloudField`
   3D texture plus noise detail) plus nebula/remnant shell emission, at
   half resolution (RGBA16F, premultiplied), then composited over the HDR target.
4. **Transparent structures**: accretion disc (5 instanced layers, premultiplied
   alpha), 276,000 deterministic GPU particles (additive point sprites), pulsar
   beams, halos and the supernova flash (additive).
5. **Lensing** (black-hole pathway only): screen-space point-mass deflection.
   The composite uses joint bilateral (depth-aware) upsampling. The volume target size
   and step count come from the render quality tier (`render/RenderQuality.h`).
6. **Post**: 6-level bloom chain (13-tap downsample with Karis average, tent
   upsample), exposure, ACES filmic tone mapping, vignette, sRGB encoding, dither.
7. **UI**: Dear ImGui panels and timeline, plus 3D labels projected from world anchors.

A per-frame uniform buffer (std140) carries the camera and all scene parameters
to every shader. GPU time is measured with `GL_TIME_ELAPSED` queries in a
4-deep ring, so the CPU never stalls waiting for results.

## 5. Resource management and errors

- Every GL object is owned by a move-only RAII wrapper (`gfx/GL.h`). The
  renderer is destroyed before the GL context.
- Shader compile/link errors are logged with the file name and compiler log.
  At startup a failure aborts with a clear message. On F5 reload the previous
  program is kept.
- `GL_KHR_debug` output is routed to the log, minus known informational IDs.
- Render targets are recreated on resize. Minimized windows skip rendering.
- The log is written to `stellaris.log` next to the executable.

---

## 6. Expansion architecture — compatible additions to the existing application

This section describes the intended architectural extension points. It is a design specification, not a claim that every listed subsystem is already implemented. The current frame pipeline, pure simulation layer, and resource ownership rules in §§2–5 remain the baseline.

### 6.1 Architectural invariants

1. Keep the scientific/scene evaluation layer independent of OpenGL. `evaluateScene(time, config)` remains the canonical deterministic entry point for presentation state.
2. Keep all mutable application state explicit: playback, selected scenario, user camera offset, render settings, and UI selection. Do not introduce hidden global state in shaders or renderer helpers.
3. Keep the default six-stage narrative functional when optional expansion settings are disabled.
4. Rendering consumes scene/model state; it must not silently modify it.
5. Preserve RAII ownership and destroy all GL resources before destroying the GLFW context.
6. Avoid building a general-purpose engine. Extend the existing renderer and pure data structures only as far as the accepted features require.

### 6.2 Proposed logical data contracts

The implementation may use different concrete names, but should preserve the following conceptual separation:

| Data contract | Contains | Must not contain |
|---|---|---|
| `ScenarioConfig` | Stable seed, selected formation preset, initial mass, rotation parameter, optional multiplicity and cloud-shape parameters | Camera exposure or bloom controls |
| `SceneState` | Time-evaluated transforms, visibility, normalized morph progress, field/particle emission controls, stage and pathway state | OpenGL handles or ImGui state |
| `StellarModelOutput` | Values produced by implemented equations and pathway classification metadata | Artistic size multipliers represented as physical radius |
| `RenderQualitySettings` | Volume resolution, ray steps/quality tier, particle density, shadow samples, post-process toggles | Scientific model parameters |
| `ScientificFieldView` | Field identifier, units/status, normalization, selected slice/plane, colour-map range | Invented units for normalized visual fields |
| `ObservationAssetMetadata` | Source, object, wavelength/filter, instrument, credit, usage terms, processing, checksum and intended role | Unverified claims about natural colour or 3D structure |

Prefer small, immutable configuration objects passed into the evaluation/render functions. Do not create a reflection/serialization framework unless a concrete scenario save/load requirement needs it.

### 6.3 Molecular-cloud environment subsystem

Extend the existing 128³ noise-volume and ray-marched volume approach instead of creating a second unrelated cloud renderer.

The environment representation should support:

- a low-frequency cloud envelope controlling the overall silhouette;
- medium-frequency turbulent structure controlling internal variation;
- high-frequency wisps used sparingly, not as uniform surface noise;
- filament masks and elongated density ridges;
- one or more dense-core masks;
- optional compact globule silhouettes;
- dust-lane attenuation and outflow cavities;
- external illumination direction and colour;
- a separate emission/ionisation contribution where the selected scene calls for it.

Keep the different contributions identifiable in code and in shader parameters. Avoid one opaque, heavily nested noise expression that no longer has controllable structure. All procedural fields must be seeded deterministically and must remain stable during timeline seeking.

A cloud preset should be data/configuration driven. Initial candidate presets are `DiffuseCloud`, `FilamentaryCore`, `BokGlobuleField`, `ClusteredCores`, and `CompressedRidge`. These are visual/scenario presets, not claims that a fully solved cloud-physics model exists.

### 6.4 Volume-rendering subsystem

The renderer should continue to produce premultiplied volume colour/transmittance and composite it consistently with the HDR scene. Improvements should be incremental and independently profileable:

1. **Sampling policy:** preserve a coarse-to-fine or adaptive step strategy only if it reduces cost without causing visible discontinuities. The step length must be bounded and based on density/optical-depth variation, not arbitrary frame jitter.
2. **Density hierarchy:** use the existing low/medium/high-frequency noise with a documented density contribution. Avoid adding detail frequencies that alias at the target pixel footprint.
3. **Lighting:** retain the current single-scattering baseline; add an ambient/multiple-scattering approximation only as a bounded approximation with a tunable and documented energy contribution.
4. **Shadowing:** reuse the existing light-space optical-depth sampling; add samples only when their measured visual benefit justifies cost.
5. **Phase function:** make anisotropy parameters material/preset driven, bounded to a stable range, and label them as rendering parameters unless they are derived from a documented dust model.
6. **Upsampling:** use depth-aware or bilateral upsampling if it improves volume edges against full-resolution geometry. Validate at the star boundary and thin filaments.
7. **Temporal accumulation:** optional. It must reject stale history after seeking, changing parameters, changing camera discontinuously, resizing, changing scenario, or shader reload. Capture/self-test paths must remain deterministic.
8. **Resolution tiers:** support a high-quality capture path and a lower-cost interactive path without changing scientific state.

Do not implement path tracing, a full radiative-transfer solver, or a complete multi-scattering solution merely to claim advanced rendering.

### 6.5 Stellar-body and atmosphere subsystem

The current star/protostar sphere should evolve into a layered emissive model, not a generic metallic PBR material stack.

Logical rendering components:

- **Photosphere:** primary emitted light, blackbody-informed base chromaticity, limb darkening, procedural granulation-like pattern, and optional starspot mask.
- **Chromosphere:** thin, warmer/cooler emission layer selected for contrast and stage context; avoid a uniformly thick glowing shell.
- **Corona:** extended, low-density luminous structure with anisotropic/structured emission. It must not appear as a perfectly uniform halo.
- **Prominences/loops:** explicitly authored or analytically generated ribbon/curve structures. Treat their shape as a visual model unless a validated field/plasma calculation is introduced.
- **Flares:** localised time-bounded emission events with deterministic activation, controlled bloom, and an identifiable origin on the stellar surface.
- **Coronal mass ejection:** an optional outward-moving shell/particle structure. It is a prescribed visual event, not an MHD calculation.

Use separate parameters for emission, absorption, opacity and bloom contribution. Do not use post-process bloom to compensate for missing surface structure. Ensure atmospheric layers respond correctly to camera distance, exposure and star size compression.

### 6.6 Typed particle and ejecta subsystem

Retain the deterministic seed/time model as the default. Extend its particle attributes conceptually with:

- stable particle ID and seed;
- phenomenon/species enum;
- origin region and launch time;
- trajectory family and phase parameters;
- size, opacity, and emission class;
- optional velocity/tangent for streak orientation;
- lifetime/fade interval;
- scenario/pathway mask.

Candidate species: collapse/infall, Keplerian disc, jet knots, stellar wind, flare/CME ejecta, planetary-nebula shell, supernova ejecta, and ambient dust. Species share buffer/render infrastructure but must not all use identical random radial motion. Each trajectory must be deterministic at arbitrary time `t`.

Only introduce a compute-shader-updated SSBO simulation if profiling shows that stateless evaluation cannot support a specific accepted interaction. If introduced, provide a deterministic fixed-step mode or a documented checkpoint/replay strategy, bounds checks, memory barriers, and CPU reference tests for a small case. Do not make the core timeline dependent on a frame-accumulated particle state.

### 6.7 Magnetic-field visualisation subsystem

Treat field lines as a scientific overlay generated from an explicit analytical/prescribed vector field. A dipole field is a reasonable first stellar example; more complex cloud or disc fields require separately documented assumptions.

- Generate reproducible seed points from configuration.
- Integrate field-line curves in a deterministic direction with a maximum path length and explicit termination rules.
- Render curves as thin camera-facing ribbons or controlled instanced segments; do not rely on unsupported wide hardware lines.
- Add tracer particles only as an optional visual aid that follows the selected field geometry.
- Allow field orientation/strength and line density to be adjusted independently from the physical stellar model.
- State in the UI that field geometry is analytic/prescribed and is not a self-consistent MHD solution.

Do not add field lines to every scene by default. Use them when they explain jets, stellar activity, or magnetic structure.

### 6.8 Supernova and remnant morphology subsystem

Extend the existing supernova flash/ejecta sequence with separable visual layers:

1. shock-front boundary;
2. inner and outer ejecta shells;
3. asymmetric large-scale expansion;
4. seeded clumps and filaments;
5. optional Rayleigh–Taylor-like fingers at selected interfaces;
6. a later remnant envelope with fading emission.

The geometry/noise may create plausible irregular morphology, but must be identified as a procedural instability-inspired representation. It is not a hydrodynamic instability solution. Preserve the pathway guard: a low/intermediate-mass white-dwarf route must not receive the core-collapse supernova sequence.

### 6.9 Scientific inspection and observation subsystem

Scientific inspection should consume the same fields and state that drive the model. It must not invent a second dataset for the UI.

Supported views may include:

- density/opacity visualization;
- temperature reference or prescribed temperature visualization;
- velocity-direction/magnitude vectors where defined;
- particle trajectories and species;
- analytic magnetic-field geometry;
- plane/cross-section slices through procedural fields;
- timeline and mass-dependent comparison.

Every field view must include a label that distinguishes physical units, normalized units, reference values, prescribed values, and artistic controls. A density map derived only from shader noise must be labelled normalized/procedural, not kg m⁻³.

Observation images should be loaded as attributed external assets. The renderer may display an image as a panel, projection, sky-plane layer, or comparison reference. Do not infer volumetric structure or physical depth from a 2D image without a documented reconstruction method.

### 6.10 Multi-scenario data flow

A scenario is a parameterized initial setup evaluated by the existing timeline/state pipeline. The core timeline remains the common presentation framework. Scenario data should determine:

- cloud morphology preset and seed;
- initial mass or masses;
- optional binary separation/orbital parameters;
- optional cluster member count/initial arrangement;
- external compression direction/time for triggered-compression presentations;
- camera path and relevant annotations.

Binary orbit and cluster arrangement may be analytically prescribed. Do not label their evolution as an N-body calculation unless one is actually implemented and validated. The default scenario must remain the current single-star path.

### 6.11 Render-pass integration and resource budget

Preserve the existing pipeline order unless a measured rendering dependency requires a change. A possible expanded order is:

1. procedural/observation sky and background;
2. opaque stellar bodies and geometric subjects;
3. depth-aware volumetric environment;
4. transparent discs and volume-adjacent structures;
5. typed particles and field-line overlays with deliberate depth/blend rules;
6. pathway-specific remnant/lensing effects;
7. HDR post-processing;
8. scientific overlays and ImGui.

Do not add a general render graph framework solely for diagrammatic elegance. A small explicit pass schedule is sufficient while the number of passes remains manageable.

Measure the memory impact of every new 3D texture, HDR target, history buffer and source image. A 4096² RGBA8 texture occupies approximately 64 MiB before mipmaps; a full mip chain increases that to roughly 85 MiB. A 4096² RGBA16F texture occupies approximately 128 MiB before mipmaps. These figures are uncompressed storage estimates; driver formats and copies may add overhead. Use compressed formats where their visual quality and data semantics permit them. Never compress scientific scalar data blindly.

### 6.12 Architecture exit criteria

The extension architecture is acceptable when:

- the existing default six-stage scenario still runs without optional systems;
- all new scene configuration is deterministic and explicit;
- the pure simulation layer remains independent of OpenGL;
- optional systems can be disabled without breaking the renderer;
- every new GPU resource has RAII ownership and resize/recreation rules;
- asset metadata is retained beside external assets;
- field overlays report their scientific status and units;
- memory and GPU time are measured for representative worst-case scenes;
- no feature requires changing the academic freeGLUT demonstration unless that requirement is explicitly updated.
