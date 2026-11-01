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
| Scientific criteria | met as documented | `docs/SCIENCE_NOTES.md`; provenance tags in the UI |
| Engineering criteria (docs, dependencies, resources, tests) | verified | README, `docs/`, RAII, tests |

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
- Scientific limitations are in `docs/SCIENCE_NOTES.md` §7. Two references
  (Sukhbold 2016, Shu 1977) are cited from memory.
