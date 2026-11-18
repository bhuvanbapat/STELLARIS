# STELLARIS — Verification record

Machine: Windows 11, NVIDIA GeForce RTX 4050 Laptop GPU (driver 621.01,
OpenGL 4.5.0 core reported), MSVC 14.44, CMake 4.4.4. Date: 2026-11-02.
Everything below was actually run. Items not verified are listed in §6.

## 1. Build

| Check | Result |
|---|---|
| Configure + Release build of all targets (`stellaris`, `stellaris_tests`, `stellaris_glut_demo`) from the root project | pass, 0 warnings in project code at /W4 |
| GLUT demo built **standalone** (`cmake -S glut_demo -B glut_demo/build`) | pass |
| Clean build in a fresh directory (`build-clean`, new dependency download) | see §1a |

### 1a. Clean-build verification

A fresh configure and build in `build-clean` (dependencies downloaded again) built all three targets, and `ctest` passed (exit 0). `build.bat` run from PowerShell also completed: configure, build, tests 100% passed.

## 2. Automated tests

**Unit tests** (`ctest`; 28 tests, no GPU needed):

- Timeline: contiguous stage table; boundary lookup; initial state and reset;
  playback only while playing; speed changes the rate, not the position;
  wall-clock steps clamped; stop at end and restart; seek/step/stage clamping.
- Stellar model: luminosity pieces and monotonicity; Sun-like values (T_eff
  within 5 % of 5772 K, lifetime 10¹⁰ yr); trends with mass; mass clamping;
  pathway categories and boundary flags; blackbody color ordering; free-fall
  order of magnitude and n^−½ scaling.
- Scene state: determinism, with unrelated evaluations interleaved; backward
  seeking equals forward evaluation; continuity of all scene parameters every
  5 ms across the whole timeline for 1, 15 and 40 M☉; both sides of every stage
  boundary; each pathway ends in its own remnant (no supernova on the
  white-dwarf path); no supernova before stage 6; rotation controls disc size;
  main-sequence color/size depend on mass; protostar larger and cooler than its
  main-sequence star; camera path smooth (max step < 0.6 units per 10 ms) and
  finite; user camera offset layered and removable; stage text complete;
  particle generation deterministic.

Result: **28 / 28 passed** (`stellaris_tests.exe`).

**In-application self-test** (`stellaris.exe --selftest`, real GPU and window):
initial frames render without GL errors; selecting each of the six stages
produces that stage; all three final pathways render; playback advances at 1×;
pause holds position; 40 rapid seeks across stage boundaries; reset restores
t = 0, paused, 1×; window resize recreates targets. Result: **0 failures**.

## 3. Performance (`stellaris.exe --benchmark`, vsync off)

| Stage sample | 1600×900 GPU / avg fps | 1920×1080 GPU / avg fps |
|---|---|---|
| 1 cloud | 4.37 ms / 192 | 5.80 ms / 164 |
| 2 collapse | 6.11 ms / 155 | 8.21 ms / 118 |
| 3 protostar | 5.93 ms / 161 | 7.97 ms / 124 |
| 4 disc + jets | 5.61 ms / 172 | 7.42 ms / 131 |
| 5 main sequence | 6.84 ms / 141 | 9.25 ms / 106 |
| 6 planetary nebula | 8.06 ms / 117 | 10.90 ms / 91 |
| 6 supernova | 8.27 ms / 116 | 11.21 ms / 89 |
| 6 NS remnant | 7.46 ms / 127 | 10.11 ms / 98 |
| 6 black hole | 9.51 ms / 100 | 13.16 ms / 76 |

Worst single frames were about 2× the average (up to 21 ms at 1080p). With
vsync on (the default), every stage stays above the 60 Hz budget on this
laptop. Particle count: 276,000. Volume: 72 steps at half resolution.

## 4. Visual inspection (screenshots taken with `--capture` and inspected)

Several review rounds fixed: overexposure, particles near the camera filling
the screen, a blocky volume edge around the star (fixed with a fixed-length
ray-march step), the disc hiding the protostar, flat-white stars,
over-bright nebula shells, the camera ending inside the supernova remnant,
the composition shift's sign, and a corkscrew artifact in the GLUT jets.
Final state:

| Stage | Observed |
|---|---|
| 1 Molecular cloud | irregular, filamentary, self-shadowed indigo/violet cloud with a readable silhouette against a star field with a faint galactic band; placed clear of the info panel |
| 2 Collapse | warm glow develops at the dense core; inner material visibly spirals inward while the outer cloud remains |
| 3 Protostar | distinct orange protostar with halo inside the flattened, swirling envelope |
| 4 Disc & jets | orange flared disc around the protostar; blue-white bipolar jets along the axis with knots and a reddish bow shock; outflow cavity in the cloud |
| 5 Main sequence | limb-darkened star; 1 M☉ warm white, 15 and 40 M☉ larger and bluer; envelope dispersing |
| 6 White-dwarf path | orange red giant with large convective cells → envelope shells → bipolar nebula (teal inside, red rim) with a bright white dwarf |
| 6 Neutron-star path | red supergiant → supernova flash → clumpy expanding remnant → neutron star with rotating beams |
| 6 Black-hole path | supergiant → weaker explosion → dark shadow, photon ring and lensed background inside the remnant |
| GLUT demo | all six phases render: point cloud, collapse, lit protostar, Keplerian disc with jets, main-sequence star, wire-sphere nebula with white dwarf; HUD text, timeline and help text visible |

## 4a. Code review

A read-only review with the PR Review Toolkit code-reviewer agent confirmed:
the std140 UBO layout matches; GL state is reset per pass; RAII and teardown
order are correct; there is no hidden per-frame state affecting the scene; no
NaN or division-by-zero sources. It found three issues, all fixed and
re-verified (build, 28/28 tests, self-test 0 failures, GLUT capture):
1. Clicking a stage label sought to the mouse position after the first frame
   instead of selecting the stage (now handled with a per-press flag).
2. The GLUT demo's fixed-function lighting had no visible effect, because the
   only lit objects were spheres containing the light. An orbiting planet now
   shows the terminator.
3. Two `smoothstep` calls with edge0 > edge1 (undefined in GLSL) were rewritten
   as `1 − smoothstep(lo, hi, x)`.

## 5. Acceptance criteria (PROJECT_SPEC §15)

| Criterion | Status | Evidence |
|---|---|---|
| Builds; launches | verified | §1, captures, self-test |
| All six stages accessible | verified | self-test stage selection; captures |
| Play, pause, resume, reset | verified | unit tests and self-test |
| Timeline seeking; direct stage selection | verified (logic + rendering); see §6 for mouse | unit tests; self-test |
| Backward/forward seeking coherent | verified | determinism and continuity tests |
| Scientific controls work | verified | mass changes color/size/pathway; rotation changes disc size (unit tests, captures) |
| Interface responsive | verified on this machine | benchmark (≥ 76 fps at 1080p) |
| Visual criteria (cloud 3D, collapse readable, distinct stages, disc vs jets, outcomes differentiated, transitions, initial composition) | inspected | §4 |
| Scientific criteria | met as documented | `SCIENCE_NOTES.md`; provenance tags in the UI |
| Engineering criteria (docs, dependencies, resources, tests) | verified | README, root documentation set, RAII, tests |

## 6. Not verified / known limitations

- **Human interaction:** the logic behind mouse clicks and drags on the timeline,
  buttons and sliders is exercised in code. Physical mouse use was not automated;
  please try it once before the presentation. Keyboard shortcuts were not
  exercised automatically either.
- Full-screen toggle, multi-monitor setups and high-DPI scaling other than this
  laptop's have not been tested.
- Only this RTX 4050 laptop has been tested. Other GPUs (especially integrated
  Intel) may be slower.
- No long-duration (hours) memory test was run. GPU resources are allocated at
  startup and on resize only.
- Visual quality is a judgment against `VISUAL_DESIGN.md` based on still frames.
  Motion was inspected only through sequences of stills and playback in the
  self-test.
- Scientific limitations are in `SCIENCE_NOTES.md` §7. Two references
  (Sukhbold 2016, Shu 1977) are cited from memory.

---

## 7. Verification plan for expansion features — not yet executed

Everything in this section is a planned verification requirement. It is **not** a record that the associated feature exists or has passed. The historical build, test and benchmark results above remain tied to the date and code state recorded in the existing entries.

### 7.1 Regression gate before every enhancement

Before a feature branch/change is considered:

- configure and build all current targets in Release;
- run `ctest` and record the count/result;
- run `stellaris.exe --selftest` where supported;
- run `stellaris.exe --benchmark` and save the unmodified output;
- capture the default six stage times and all three final pathways;
- verify timeline seeking, rewinding, speed, reset and camera reset;
- confirm the separate GLUT demonstration still builds and runs if its files or shared build configuration changed.

A baseline failure blocks the new feature until it is understood. Do not use the new feature's branch to mask an unrelated failure.

### 7.2 Determinism tests for new procedural systems

For each new cloud, particle, field-line, prominence, flare, or remnant-morphology system:

1. Evaluate the same scenario, seed, time and parameters twice and compare the generated CPU state or deterministic output checksum where practical.
2. Evaluate times in a different order (for example, 12 s, 90 s, 42 s, then 12 s again) and confirm the repeated timestamp matches.
3. Compare direct seeking with normal playback to the same time.
4. Repeat after reset and after changing a non-related UI setting.
5. Test each stage boundary immediately before, at and after the transition.
6. Change scenario seed and verify the intended variation without changing other unrelated state.

Temporal accumulation, if introduced, requires a separate test proving history is invalidated after seek, reset, camera discontinuity, scenario/parameter change, resize, shader reload and capture-mode activation.

### 7.3 Molecular-cloud verification matrix

| Test | Method | Pass condition |
|---|---|---|
| Silhouette hierarchy | Wide/medium/close fixed-camera captures | Envelope, core and filament network are identifiable; no single spherical fog shell dominates |
| Density boundedness | Inspect/debug sample the procedural field over the domain | Output finite and within documented bounds; no NaN/Inf |
| Seed reproducibility | Re-run with identical seed/config/time | Same structure and stage state |
| Seed variation | Change only seed | Structure changes but remains within intended visual envelope |
| Temporal stability | Seek backward/forward across collapse | No swimming, random regeneration or discontinuous detail |
| Alias resistance | Compare at target resolution and reduced resolution | Fine detail fades rather than flickering or forming moiré |
| Compositing | Place cloud in front of/behind opaque star geometry | Correct depth termination and no leakage halos |

### 7.4 Volume-rendering verification matrix

- Compare baseline and modified renderer at identical time, camera, exposure and output resolution.
- Record GPU time, ray-step count distribution and output resolution for each quality preset.
- Inspect optical-depth/opacity accumulation for finite values and stable limits.
- Test very low and high density settings to detect over-brightness or premature early exit.
- Test external light and central-star light independently.
- Test silhouette edges against the full-resolution depth buffer.
- Inspect banding, noise, interpolation, half-resolution upsample, and near-camera intersections.
- Test resize, minimize/restore and camera changes.
- If temporal history exists, test ghosting and history invalidation explicitly.

Pass condition: a measurable quality improvement or equivalent visual quality at lower measured cost, with no regression in deterministic capture and no broken premultiplied-alpha compositing.

### 7.5 Stellar-atmosphere verification matrix

| Test | Pass condition |
|---|---|
| Photosphere colour | Existing mass/temperature colour ordering remains correct; no unexpected hue shift from exposure changes |
| Limb darkening | Surface brightness changes smoothly toward the limb under the specified model |
| Granulation/spot stability | Pattern is deterministic and does not swim with timeline seeking |
| Chromosphere/corona separation | Both layers are distinguishable without a thick opaque halo |
| Prominence anchoring | Curves remain attached to the intended surface locations under rotation |
| Flare timing | Event starts/peaks/fades at deterministic times and does not affect unrelated pathways |
| HDR behavior | No persistent full-disc clipping or uncontrolled bloom |
| Transparency/depth | Shells do not pop, sort incorrectly or obscure the surface |

### 7.6 Particle verification matrix

Test each species separately and in combined worst-case scenes. Verify stable ID, trajectory, lifetime/fade, size, blend mode, stage mask and performance. Specifically inspect:

- sub-pixel coverage and shimmer;
- near-camera fading;
- axial symmetry and jet alignment;
- differential rotation in the disc;
- species cross-contamination at timeline boundaries;
- additive overdraw and resulting HDR saturation;
- deterministic output at arbitrary seek times;
- memory bounds and buffer indexing.

If the particle system changes from stateless evaluation to compute-updated state, add a CPU-reference validation test, bounds-check test, memory-barrier correctness test and repeatability test before accepting the change.

### 7.7 Magnetic-field verification matrix

- Compare the implemented field vector at selected test coordinates against hand-calculated/reference values.
- Verify coordinate axes, dipole orientation and sign convention.
- Verify the line integrator terminates at the documented bounds and handles singular/near-zero field values.
- Verify seeded lines are stable under repeated evaluation.
- Verify that the rendered curve follows the computed vector direction and does not become a particle-trajectory claim.
- Confirm the UI labels the field as analytic/prescribed and uses correct units or normalized display wording.

### 7.8 Supernova/remnant verification matrix

- White-dwarf pathway must not activate core-collapse supernova features.
- Neutron-star/black-hole pathway guards remain consistent with the documented simplified categories and uncertainty warnings.
- Shock-front, ejecta and remnant envelope activation times are ordered and deterministic.
- Shell perturbations remain stable at fixed seed/time.
- No full-frame exposure blowout hides the remnant structure.
- No mesh/particle discontinuity occurs when seeking across event start/end.
- The UI and documentation identify instability-inspired geometry as procedural if no fluid solver exists.

### 7.9 Scientific inspection verification matrix

For each enabled field view, confirm:

- the displayed data come from the same model/render field as the visual scene;
- the range/normalization is explicit;
- units are correct or the display says `normalized`/`prescribed`;
- cross-section values agree with direct field sampling within the selected sampling tolerance;
- toggling the view does not alter the simulation state;
- cinematic mode can be restored without stale overlays;
- annotations remain legible and do not obscure the focal subject.

### 7.10 Observation-asset verification matrix

No observation image may enter a release asset bundle until the following have been reviewed:

- stable source page/archive identifier;
- source institution and creator/credit;
- asset-specific usage/redistribution terms;
- instrument and wavelength/filter/band, when supplied;
- false-colour/composite channel explanation;
- original resolution and local derivative dimensions;
- derivative/crop/colour-processing record;
- checksum and local path;
- offline load test;
- visible attribution where required.

A public archive URL alone is not proof that the image may be repackaged. A 2D image must not be presented as a volumetric reconstruction unless the project has a documented reconstruction method and source data.

### 7.11 Scenario and comparison verification

For each optional scenario, test reproducibility from the same configuration, correct camera framing, no state leakage after returning to the default scenario, valid stage/pathway transitions, and no unbounded entity/particle allocation.

For comparative stellar tracks, test units, input domain, interpolation endpoints, monotonicity where expected, no silent extrapolation, distinct legend/source labels, and uncertainty notes. Compare the current formula-based model with reference tracks only as separate clearly named models.

### 7.12 Performance acceptance and reporting

Measure at a stated resolution (default reference: 1920×1080), quality preset, GPU/driver, stage, mass/scenario and window state. Report:

- average frame time and FPS;
- median and 95th-percentile frame time where measurement support exists;
- GPU time by major render pass;
- particle count and approximate overdraw-sensitive workload;
- volume resolution and ray-step policy;
- resident VRAM or a documented estimate if direct measurement is unavailable;
- screenshot/capture settings;
- whether VSync is enabled.

Use representative worst-case tests: densest cloud, brightest stellar atmosphere, maximum particle species combination, supernova/remnant, and black-hole lensing. Do not report only the easiest scene. The 60 FPS interactive objective is a target to be measured, not a guaranteed result. If a High/Capture preset runs slower, report that honestly and keep the Balanced/Performance path usable.

### 7.13 Evidence and completion rule

A feature can move to `implemented and verified` only when the applicable tests above are run against the actual executable, results are recorded, visual evidence is retained, and the related current-state documentation is updated. Planned checks, code review, successful compilation and attractive screenshots alone are insufficient.

---

## 8. Expansion verification results (recorded 2026-11-08)

Results in this section belong to the code state of the commit that records them.
§§1–6 remain the historical record for 2026-11-02.

### 8.1 Entry gate (IMPLEMENTATION_PLAN §19.1), before any expansion change

Release build clean; unit tests 28/28; `--selftest` 0 failures; 1920×1080 benchmark
(vsync off) 76–168 fps, GPU 5.91–13.19 ms (cloud 5.91, collapse 8.35, protostar 8.08,
disc 7.58, main sequence 9.44, WD nebula 10.95, supernova 11.28, NS remnant 10.25,
black hole 13.19). This matches the 2026-11-02 record. Baseline captures of the
standard review set (six stages; 15 and 40 M☉ endings) were taken for comparison.

### 8.2 Workstream A — structured molecular cloud (implemented and verified)

- Unit tests (7 new; 35/35 total): same seed is bitwise identical; different seeds
  differ; all values finite, non-negative and ≤ 1.25; the dense core is > 0.95 at the
  hub and > 2× the mean at r = 30; filaments are strong on their centerlines and
  weak 10 units sideways; the baked grid equals exact evaluation at voxel centers;
  ambient particles sit in > 2× denser field than uniform samples.
- Visual: wide shots for seeds 1, 2 and 3 show distinct envelopes, filament
  streaks converging on the core, a darker dense core, and rim-lit cometary
  globules in silhouette. Collapse, protostar and disc stages still read. No step
  banding from empty-space skipping was visible in the standard review set.
- First implementation measured 15.8–28.1 ms GPU at 1080p, over budget. After
  coarse shadow density, central-shadow skip and shell-aware empty-space skipping:
  cloud 6.56, collapse 7.67, protostar 7.44, disc 6.96, main sequence 7.42,
  WD nebula 12.80, supernova 11.75, NS remnant 12.80, black hole 9.22 ms
  (78–151 fps). Stages 1–5 are faster than the baseline; stage-6 nebula/remnant
  views are ~1.5–2.5 ms slower than baseline but within the 60 fps budget.
- Stage-5 haze regression found and fixed: the feedback-clearing term now reaches
  further and starts earlier (80–96 s), and the stage-5 camera was raised to 18–20°.
- Cloud bake time 445–580 ms per seed change (logged at startup and on change).
- Not yet done for A: a reduced-resolution aliasing comparison (planned with the
  quality tiers in Workstream B/I).

### 8.3 Workstream B — volume refinement and quality tiers (implemented and verified)

- Depth-aware upsampling: protostar limb at t = 52 s cropped at full resolution in
  Performance (⅓-res volume), Balanced (½) and Capture (full-res reference). The
  limb is equally crisp in all three, with no cloud halo across it. Foreground gas
  over the lower half matches the full-resolution reference.
- Quality tiers, 1920×1080, vsync off (stages: cloud, collapse, protostar, disc,
  main sequence, WD nebula, supernova, NS remnant, black hole):
  - Performance: 2.88 3.40 3.42 3.25 3.54 5.51 5.08 5.47 4.07 ms (180–327 fps)
  - Balanced (default): 6.61 7.77 7.49 7.17 7.26 12.79 11.73 12.79 9.17 ms (78–149 fps)
  - High: 14.38 17.97 17.16 16.08 15.26 27.37 25.10 26.76 20.62 ms (36–69 fps).
    **Below 60 fps** on this GPU for collapse, protostar and all stage-6 views.
  - Capture: 25.4–50.1 ms (20–39 fps), intended for offline screenshots.
- Tiers change only renderer settings (`Renderer::setQuality`); `SceneState`,
  `ModelConfig` and the timeline are untouched by construction. The particle subset
  for Performance is a fixed hash of the particle index, so it is stable over time.
- Unit tests 35/35 after the change.
- Not done: temporal accumulation (P3), and a multiple-scattering approximation.
  Neither was needed for the visible failure modes found so far.

### 8.4 Workstream C — layered stellar atmosphere (implemented and verified)

- Unit tests (2 new; 37/37): activity, flare and atmosphere are present for 1 M☉ on
  the main sequence, and activity is 0 for 15 M☉ (no outer convection zone) while its
  smooth atmosphere remains. Neither appears before the main sequence or as a
  giant. The flare is a single event peaking at 91–92 s, zero before, < 0.01 after
  105 s, and repeatable. The new fields pass the 5 ms continuity test.
- Visual, full-resolution crops around the star: at t = 97 (1 M☉) granulation, dark
  spots, the chromosphere rim and prominence arches beyond both limbs are visible.
  At t = 91.8 the flare is a localized kernel at the active region, not a full-frame
  flash. 4 and 15 M☉ stars are blue-white, smooth and limb-darkened, with no spots.
- Defects found and fixed during review: flare site rotated away from the camera at
  flare time (spot layout changed); edge-on prominences looked like "horns" (wider
  span, taller, thicker); leftover disc washed out the star (disc/jets now fade by
  89–91 s); hot stars washed out (equal surface brightness, stage-5 exposure
  0.75 → 0.6); a bright prominence ring over the disc (emission suppressed in front
  of the photosphere).
- Regression gate: standard review set intact; `--selftest` 0 failures; 1080p
  Balanced 6.48–12.76 ms (78–154 fps), main sequence 6.48 ms (was 7.26).
- Not done: CME-like ejection (optional per spec), and multi-distance aliasing
  inspection beyond the authored camera distances.

### 8.5 Workstream D — typed particle species (implemented and verified)

- New stellar-wind family (14,000 particles, population 4) and episodic jet knots.
  Particle generation stays deterministic (existing tests, 37/37).
- Visual crops: at t = 72 s the jets show compact soft knots inside a faint
  collimated stream. At 96 s (15 M☉) the hot-star wind shows as sparse outward
  flecks. At 112 s (1 M☉) the giant's dusty mass-loss wind is reddish.
- Defects found and fixed: knots first drew S-shaped curves (all coordinates came
  from one random number) and then boxes (uniform offsets). They now use
  independent Gaussian offsets.
- Regression: `--selftest` 0 failures; 1080p Balanced 6.48–12.81 ms (78–153 fps),
  unchanged within noise.
- Not done: velocity-aligned streaks, species toggles in an inspection UI (with
  Workstream G), and CME ejecta.

### 8.6 Workstream F — structured supernova and remnant (implemented and verified)

- Sequence captured for 15 M☉ at 114.9, 116, 118, 122, 132 and 147 s: flash; hot
  ejecta with radial fingers; a distinct, broken shock rim with stratified ejecta
  and fingers inside; a persistent asymmetric remnant with the neutron star and
  beams. Also checked for 40 M☉ (weaker remnant + lensing) and 1 M☉ (no supernova).
- Systematic debugging record: the shock first rendered as a uniform "bubble".
  Hypothesis 1 (the outer particle layer causes it) was **rejected**: changing it had
  no effect. Disabling the volume shell emission removed the bubble, which located
  the cause. The filament mask thresholded the ridged noise channel, whose values
  lie mostly above the threshold, so the mask was ≈ 1 everywhere. Fixed by
  thresholding the fBm channel across its middle and dimming the front.
- The NS ending camera was pulled back (56 → 76) so the larger asymmetric remnant
  stays in frame.
- Regression: unit tests 37/37; `--selftest` 0 failures; the standard review set is
  intact (WD pathway unchanged). 1080p Balanced: supernova 13.88 ms, NS remnant
  14.61 ms, black hole 11.11 ms (68–89 fps; +2–2.5 ms, because the remnant interior
  is now sampled for ejecta emission). Other stages unchanged.

### 8.7 Workstream E — analytic magnetic-field overlay (implemented and verified)

- Unit tests (3 new; 40/40): field vectors equal hand-computed values (axis r = 2:
  (0, 0.25, 0); equator r = 2: (0, −0.125, 0); r⁻³ falloff; zero at the singular
  point). Every integrated point of closed lines from 30°, 45° and 60° footpoints
  satisfies the analytic dipole relation r = L sin²θ to < 0.2 %. Lines close at the
  mirrored latitude (±0.002) with apex r = L (±2 %). Generation is deterministic,
  bounded by maxSteps/maxRadius, and high-latitude lines are open.
- Debugging record: the first run failed the 1 % tolerance. A diagnostic showed
  zero error along every line body and 1.1–1.5 % only at the end point, caused by a
  radial snap onto the surface. Fixed in the integrator (bisection along the last
  step). The test was tightened to 0.2 % on every point.
- Visual: 1 M☉ at 97 s shows thin closed loops and open polar lines, hidden behind
  the star; 15 M☉ at 140 s shows the tilted neutron-star field aligned with the
  beams. The model label is visible at the bottom. 7,984 + 34,672 segments.

### 8.8 Workstream G — scientific inspection (partially implemented)

- **Implemented and verified:** Cinematic/Scientific view switch; density
  cross-section evaluated by the shared `cloud_density.glsl` (the exact function the
  volume integrates, so the "cross-section agrees with the volume field" check holds
  by construction); viridis map with a legend from the same polynomial; units shown
  as "normalized ρₙ (dimensionless)". Captured at 8, 34 and 52 s: envelope, filaments
  and the core concentrating during collapse are visible. The legend range was
  widened from 2.5 to 4 because the core saturated a large area.
- **Partial:** no temperature/velocity overlays (the model owns no such fields beyond
  prescribed trajectories, so none are invented) and no per-species particle toggles.
- **Deferred (blocked): observation / multi-wavelength mode.** It requires downloading
  external astronomical images, which needs the user's explicit approval and an
  asset-by-asset rights review. Nothing was downloaded.
- Regression after G: see §8.9.

### 8.9 Regression after the 2026-11-08 expansion round

- Unit tests 40/40; `--selftest` 0 failures; full Release build of all three targets
  (main app, tests, GLUT demo) with no warnings in project code.
- Standard review set (six stages; 15 and 40 M☉ endings) captured and inspected:
  all stages intact.
- 1920×1080, vsync off (stages: cloud, collapse, protostar, disc, main sequence,
  WD nebula, supernova, NS remnant, black hole):
  - Performance: 2.91 3.37 3.36 3.22 2.92 5.53 6.06 6.15 4.91 ms (162–339 fps)
  - Balanced: 6.64 7.74 7.47 7.20 6.49 12.84 13.89 14.63 11.12 ms (68–153 fps)
  - Balanced + Scientific view: 6.81–14.87 ms (67–146 fps)
- Particle count 290,000; cloud structure texture 16 MiB (128³ RGBA16F); the
  resident VRAM total was not measured directly (no driver query used).

### 8.10 Expansion acceptance gate (PROJECT_SPEC §18.14) — status on 2026-11-08

| Gate item | Status | Evidence |
|---|---|---|
| Six-stage sequence still builds, launches, plays | verified | §8.9, self-test |
| Seek/rewind/reset/speed/stage selection deterministic | verified | unit tests (determinism, continuity), self-test |
| Cloud morphology stable at arbitrary times | verified | seeded CPU field (bitwise test); time only enters through the prescribed collapse mapping |
| Volume: edges, banding, compositing | verified (inspected) | §8.2, §8.3 limb crops |
| Stellar layers distinct under HDR | verified (inspected) | §8.4 |
| Particle species distinct, repeatable, no excessive overdraw | verified (inspected); overdraw not measured separately | §8.5 |
| Magnetic lines use an identified analytic model | verified | §8.7, on-screen label |
| Supernova morphology not claimed as hydrodynamics | verified | SCIENCE_NOTES §3, code comments |
| Overlays show real fields with units/normalization | verified for the density cross-section | §8.8 |
| Observation assets with provenance | verified (added after approval) | §8.11, provenance check |
| Optional scenarios do not break default | **deferred** (binary, cluster, triggered compression not implemented; mass presets 1/15/40 M☉ already existed) | §8.12 |
| Comparative laboratory sources | **partially implemented, verified**: simplified-model table only; no reference tracks | §8.12 |
| Performance measured at a stated resolution/preset | verified | §8.3, §8.9 |

### 8.11 Observation view (implemented and verified, 2026-11-08)

- The user approved downloads. Seven images (1.9 MB total) were retrieved from the
  ESA/Hubble, ESA/Webb, ESO and NASA SVS CDNs into `assets/observations/`. Each
  source page and institutional licence page was read before downloading (CC BY 4.0
  ×6, public domain ×1).
- `tools/check_observations.py` (CTest test `observation_provenance`): 7 records,
  7 images, 0 problems. All required fields are present, the SHA-256 checksums match,
  and the JPEG header dimensions match the records. Every image has a sidecar.
- Unit tests (4 new; 44/44): flat-JSON parser (escapes, malformed input); the catalog
  loads all 7 records with credit/licence/band; stage and pathway linkage (two-band
  Pillars pair for stages 1–2, HH 211 for 3–4, SDO for 5, one image per final
  pathway); records without credit are rejected.
- Visual: Observation view captured for stages 1, 4, 5 and all three endings. Images
  display at native aspect ratio with the credit directly below, and instrument, band,
  image type, licence, source and caption are legible at 1600×900. GPU memory 3.9–8.7
  MiB per image, loaded on first use (logged).
- `--selftest` passes with 0 failures in both the default and the Observation view.
- Not verified by clicking: the two-band toggle button for the Pillars pair. Its data
  linkage is unit-tested; the button itself was not exercised automatically.
- One recorded discrepancy: for weic2216a the source page lists 1200×1200 px as the
  original, but the screensize file served is 1280×1280 px. Recorded verbatim in the
  sidecar.

### 8.12 Workstream H: mass-comparison table (partially implemented, 2026-11-08)

- Implemented: a "Compare masses" toggle (`--compare`) opens a table with the existing
  simplified relations evaluated at 1, 2, 5, 15 and 40 M☉, plus the current mass:
  L, R, T_eff, blackbody colour, main-sequence lifetime and final pathway. It is
  labelled "simplified analytic scaling relations, not evolution tracks". Clicking a
  row sets that mass. The table is display only: it does not change the model.
- Test `comparison_masses_are_in_domain_and_ordered`: every mass is inside the supported
  1–40 M☉ domain; L and T_eff rise and the lifetime falls monotonically; all three
  pathways are represented. 45/45 unit tests and the provenance test pass. `--selftest
  --compare` reports 0 failures (GPU ~5.4 ms).
- Visual: captured at 1600×900 with 3 M☉. The values match the info panel
  (65.5 L☉, 10427 K, ~5×10⁸ yr) and the table is legible without covering the
  subject.
- Deferred: binary, cluster and triggered-compression scenarios. Each needs a
  multi-subject renderer, camera composition and new prescribed dynamics (P2 cost,
  regression risk to the verified single-star path). Reference tracks (MIST/PARSEC)
  are also deferred: they need dataset provenance and a download approval.

### 8.13 Realism and energy pass (implemented and verified, 2026-11-08)

- User report: jets looked like broken, periodic streams. Cause: the Workstream D
  knot code moved half of the jet particles into 9 separate clumps. Fixed by
  restoring the continuous beam; knots are now brightness only.
- Changes: faster jets with a hot spine; motion streaks for infall, disc, jets, wind
  and ejecta; three accretion bursts; compressional core glow; dust albedo 0.5,
  H-alpha rims and [O III] envelope; residual cloud 18% → 8%; extinction 0.26 → 0.32.
- Tests: 46/46 unit tests (new `accretion_bursts_only_while_a_protostar_accretes`;
  `accretionBurst` added to the continuity check) and the provenance test pass.
  `--selftest` reports 0 failures.
- Visual: standard review set captured before and after at 1600×900. Jets are
  continuous with streaks; the core heats visibly between t = 24, 30 and 36; the
  15 M☉ supernova at t = 116/118/121 expands with radial streaks.
- Benchmark (1080p, default tier): 4.9–11.0 ms per stage. The worst case is the NS
  remnant at 11.0 ms (91 fps), within the earlier 6.6–14.6 ms range.

### 8.14 Real Milky Way sky (implemented and verified, 2026-11-08)

- Downloaded after the user approved it: ESO eso0932a, the 6000×3000 large JPEG (8.2 MB).
  Licence checked on ESO's copyright page (CC BY 4.0, visible unaltered credit,
  modifications indicated); no exception is noted on the image page. Sidecar
  `assets/sky/eso0932a.json`; the provenance checker now covers `assets/sky` (8 records,
  8 images, 0 problems).
- Visual: the review set at 1600×900 shows the Milky Way band with dust lanes and the
  bulge behind the cloud in stages 1–3. The credit "Milky Way: ESO/S. Brunier (CC BY 4.0)"
  is legible at the top (UI shown) or bottom (UI hidden). No seam line was seen.
- `--selftest` passes with 0 failures with and without `--procedural-sky`. Benchmark per
  stage: 4.8–11.0 ms, unchanged within noise. GPU memory: the texture is 91.6 MiB (logged);
  2.3 GB of 6 GB was in use during the run (nvidia-smi).
- Not done: larger versions of the 7 Observation images. The panel displays at about
  700 px and the current files are 1024–1280 px, so they would add memory and no
  visible detail.

### 8.15 Enhancement round: UI, stellar surfaces, late stages, jets, sonification (2026-11-17)

Per the user's instruction this round has **no screenshot verification**: the user
inspects the visuals. Checks run:
- Build: Release, MSVC /W4, no new warnings. New dependency: miniaudio 0.11.25, pinned by
  tag and SHA-256 (ac7af4de...c55c89).
- Unit tests 49/49 (3 new: solar asteroseismic values reproduced exactly; sonification
  per stage and deterministic; audio output finite, ceiling <= 0.85 with every voice at
  full level, and exactly silent after muting). Provenance test passes.
- `--selftest`: 0 failures, so all changed shaders compile and render every stage and
  pathway without GL errors. Benchmark (1600×900): 4.7–11.3 ms; the PN stage went from
  9.7 to 11.3 ms (knot and halo emission).
- Audio: an interactive run opened the default device ("Speakers (Realtek(R) Audio)",
  48 kHz stereo) with `--mute` and exited cleanly.
- **Not verified:** the visual result of the new photosphere, nebula, jets and UI layout
  (left to the user); how it sounds on speakers (the run above was muted); clicking the
  speaker button and volume slider.

### 8.16 Volume-first reconstruction (2026-11-18)

No screenshot verification in this round, per the user's instruction; the user inspects
the visuals. One data check was made on the baked Cas A volume: a maximum-intensity
projection shows the expected morphology (Si/Fe shell, NE/SW jets).
- Build OK; unit tests 49/49 (`streamers` and `giantAtmosphere` added to the continuity
  test); provenance checker 9 records, 0 problems (`assets/data/casa` added).
- `--selftest`: 0 failures, so the new shaders compile and every stage and pathway renders
  without GL errors. The Cas A volume loads (192³, 27 MiB).
- Benchmark at the new default (High, 1600×900), after bounding the streamer slab:
  cloud 10.3, collapse 13.4, protostar 14.3, disc+jets 17.5, main sequence 11.4, WD nebula
  24.4, supernova 23.1, NS remnant 23.9, black hole 17.9 ms (41–97 fps). The first,
  unbounded version measured 64 ms in the disc+jets stage.
- Not verified: how any of it looks; sound on speakers; the new tracer checkbox.

### 8.17 Reference library and real solar data (2026-11-18)

- **Acquisition** (`tools/acquire_assets.py`, run once): 75 OK, 10 UNAVAILABLE (no result
  titled with the object: NGC 2264 on ESO, AFGL 5180 (Webb, Hubble), HH 46/47 and HH 24 on
  Webb, R Doradus, WOH G64, a generic red-giant query, Helix on ESO, Eskimo), 1 SKIPPED
  (duplicate), 0 FAILED. Six results were reclassified by hand and logged as RECLASSIFIED.
  No substitutions.
- **Provenance check:** 73 records, 71 images, 0 problems.
- **Build and tests:** build OK; 49/49 unit tests, including two observation tests
  updated: the 7 curated links are unchanged, and no library image is linked to the wrong
  ending.
- **Self-test:** 0 failures in the default and Observation views. 70 catalogue entries
  load. Images load one at a time (16–48 MiB each, at most 6 resident). The magnetogram
  (2048×1024) and the Cas A volume load.
- **Data check:** the strongest region of the baked magnetogram lies at 19.6° S; AR 13664
  was observed at ~S18. Maximum |Br| is 2328 G.
- **Not verified:** the look of the magnetogram-driven star, the corona and the sharper
  cloud (left to the user); clicking the ←/→ navigator.

### 8.18 Reviewed on screen: jets, clouds, star (2026-11-18)

After the user reported no visible change, renders were reviewed on screen (contact
sheets and crops) and fixed iteratively:
- **Jets:** a narrow chain of knot bows replaced the faint thread / cone; the striping
  artefact is gone.
- **Protostar:** the cell net is hidden.
- **Clouds:** the smooth tubes now have crisp, eroded silhouettes against HII emission.
- **Star:** the visible / 304 Å / 171 Å views were each compared with SDO image
  appearance.

Checks: unit tests 51/51 (new: evolution phases contiguous and pathway-correct; helium
flash only below 2 M_sun); `--selftest` 0 failures. Benchmark at High, 1600×900: cloud
29, collapse 33, protostar 33, disc+jets 37, main sequence 34, endings 20–27 ms
(27–51 fps). The finer cloud sampling roughly doubles the cost of stages 1–5.

## 8.19 Fresh-clone check (2026-11-18)

Cloned https://github.com/bhuvanbapat/STELLARIS into an empty temporary folder and followed README §1–2 exactly.
- First attempt: **checkout failed on Windows** ("Filename too long", 260-char path limit) because of four
  over-long observation filenames. Fixed in 8707298: names shortened to ≤ 60 characters (stable hash suffix,
  generator updated) and one mis-filed CC BY-NC landscape photo removed. Provenance check: 76 records, 0 problems.
- Second attempt: clone OK; `cmake` configure + Release build OK (0 errors; one C4505 warning inside freeGLUT's
  own header); Cas A volume baked (192³, 27.0 MiB); `ctest`: 2/2 passed; `stellaris.exe --selftest --no-audio`:
  0 failures, 73 attributed observation images, magnetogram, Cas A volume and ESO sky all loaded.
