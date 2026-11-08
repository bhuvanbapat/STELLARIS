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
