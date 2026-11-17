# STELLARIS — Development Log

Running record of decisions and milestone results, so that work can resume
after context compaction or in a new session. Newest entries at the bottom.
Status words: **verified** (built/run/inspected), **written** (code exists,
not yet verified), **planned**.

---

## 2026-11-02 — Session 1

### Environment audit (verified)

| Item | Found |
|---|---|
| OS / GPU | Windows 11, NVIDIA RTX 4050 Laptop (6 GB), driver 621.01 |
| Compiler | Visual Studio 2022 Build Tools 17.14, MSVC 14.44 (not on PATH; used via the CMake VS generator) |
| Windows SDK | 10.0.26100.0 |
| Build system | CMake 4.4.4 (generators: VS 2022 / VS 2026) |
| Other | Git (Git Bash), Python 3.13, Node 24; clang-format 19.1.5 bundled with VS (not on PATH); no clangd, no glslang |
| Graphics libraries preinstalled | none (no GLUT/GLFW/GLEW) |

### Academic requirement (resolved by the user)

The faculty requires a genuine GLUT/OpenGL demonstration; it does **not**
require the main application to use GLUT. Decision: an independently buildable
freeGLUT demo in `glut_demo/`, separate from the main app.

### Architecture decision (ADR-001)

- **Main app:** C++20, OpenGL 4.5 core, CMake + VS 2022 generator.
  GLFW 3.4 (window/input), glad 2.0.8 (GL loader, generated once and vendored
  in `third_party/glad`), GLM 1.0.1 (math), Dear ImGui 1.91.5 (UI).
  Dependencies are fetched by CMake FetchContent from pinned release tags on
  first configure; afterwards builds need no network.
- **Why:** full shader control for volumetric ray marching, GPU particles,
  HDR + bloom; builds with the installed toolchain; shares the OpenGL
  foundation with the GLUT demo. Game engines (Unity/Unreal/Godot) were
  rejected: large installs, weaker demonstration of CG techniques for a CG
  course. WebGL/three.js was rejected: the spec asks for a native desktop app.
- **Layers:** `app/src/sim` (pure C++, no GL: timeline, stellar model, scene
  state, camera path, stage text; unit-tested) → `app/src/gfx` (GL RAII,
  shaders, meshes) → `app/src/render` (passes) → `app/src/ui` (ImGui) →
  `app/src/app` (window, loop, input).

### Determinism model (ADR-002)

`SceneState evaluateScene(time, config)` is a pure function. Particles are
evaluated in closed form in the vertex shader from (initial position, seed,
timeline time); nothing is accumulated per frame, so seeking/rewinding is exact.
The camera is an authored Catmull-Rom path of `time`; user orbit/zoom is a
separate offset removed by Recenter/Reset. Timeline: 6 stages, 148 s at 1×
(0/18/40/58/82/100/148 s). Speeds 0.25–4×. Reset = t 0, paused, 1×.

### Scientific model decisions (ADR-003) — sources checked this session

- Mass–luminosity (piecewise): L = M^4 (0.43–2 M☉), 1.4 M^3.5 (2–55 M☉),
  32000 M (>55) — Salaris & Cassisi 2005; Duric 2004 (via Wikipedia
  "Mass–luminosity relation").
- Mass–radius: R = 1.05 M^0.94 (M < 1.66), 1.37 M^0.54 — Demircan & Kahraman
  1991 as reported by Camposeo 2025 (arXiv:2508.08845); ~6 % discontinuity at
  the break is accepted for an illustrative model.
- T_eff from L = 4πR²σT⁴ scaled to T☉ = 5772 K (IAU 2015 Resolution B3).
- Lifetime t ≈ 10¹⁰ yr · M/L — order of magnitude; overestimates the most
  massive stars (noted in UI and docs).
- Pathways: < 8 M☉ white dwarf; 8–25 M☉ neutron star; > 25 M☉ black hole —
  **illustrative**. Literature: WD upper limit ~6–12 M☉ depending on
  metallicity; NS/BH outcome is not monotonic in mass (Sukhbold et al. 2016,
  "islands of explodability"). UI flags masses near the boundaries.
- Free-fall time t_ff = sqrt(3π / 32Gρ), illustrative n(H₂) = 10⁴ cm⁻³ → ~3×10⁵ yr.
- Blackbody colors: Planck × CIE 1931 analytic fit (Wyman, Sloan & Shirley 2013).
- Mass range 1–40 M☉ (lower-mass stars have not yet left the main sequence).

### Claude Code setup (this session) — see CLAUDE_SETUP.md

### Milestone status at end of this entry

- Build system, dependency module, glad: **written** (not yet configured/built).
- `sim/` layer (Timeline, StellarModel, SceneState, CameraPath, StageContent): **written**.
- `gfx/` (GL RAII, Shader with #include, Geometry), `core/Log`: **written**.
- Shaders written: common, noise3d.comp, fullscreen, sky, volume, composite,
  star, glow, disc, particles. **Not yet written:** beam, lens, bloom, final.
- Not yet written: Renderer, ParticleSystem, Ui, App/main, app/tests/glut_demo
  CMakeLists, unit tests, GLUT demo, README.
- Next: finish shaders → Renderer → App/UI → first build → screenshot-based
  visual verification per stage → tests → GLUT demo → docs.

---

## 2026-11-02 — Session 1, later: implementation milestones

**Verified this session** (details and numbers in `VERIFICATION.md`):

- Renderer, UI and app complete; all six stages and all three final pathways
  render on the RTX 4050. Unit tests 28/28. In-app `--selftest` 0 failures.
  `--benchmark` 76–192 fps depending on stage and resolution.
- Visual review loop (capture → inspect → fix), five rounds. Main fixes: HDR
  emission rescaled (everything was overexposed), particle near-camera fade and
  size cap, fixed-length volume steps (removed banding at the star's
  silhouette), thinner disc absorption, star intensity compressed, diffuse
  shell emission for the nebula/remnant, final-stage camera reframing, sign of
  the composition lens shift, ImGui speed-row alignment.
- freeGLUT demo builds standalone and from the root. All six phases captured.
  Root cause of initially black captures: reading the back buffer before the
  window had been shown (fixed by capturing after 20 timer ticks).
- Added `--capture`, `--selftest`, `--benchmark`, `build.bat`, `run.bat`,
  `run_glut_demo.bat`, README and docs.

**Decisions:**
- Particle budget 276k (90k ambient, 110k core, 16k jet, 60k ejecta);
  brightness per particle kept low so the additive sum reads as a smooth density.
- Composition: the subject is shifted right with an off-axis projection while
  the info panel is visible (eases back when the UI is hidden with H).
- Black-hole and remnant sizes are symbolic and labeled "(enlarged)".

**Open items / ideas:** stage-1 cloud could be more dramatic (more contrast
and depth layers); planetary-nebula shape is a little regular; physical mouse
interaction with the UI still needs one human check.

## 2026-11-07 — Documentation-only visual expansion specification

### Purpose

The existing STELLARIS documentation was extended to define a controlled next phase of visual/scientific expansion while preserving the implemented application and its six-stage single-star narrative.

### Documentation updated

- `CLAUDE.md`: expansion priorities, status language, deterministic behaviour, provenance requirements, performance discipline and no-duplicate-documentation rule.
- `PROJECT_SPEC.md`: detailed P0–P3 feature scope and acceptance criteria for cloud structure, volume rendering, stellar layers, particle species, magnetic fields, supernova morphology, scientific inspection, observation integration, optional scenarios and comparison.
- `VISUAL_DESIGN.md`: expanded art direction for cloud morphology, stellar atmospheres, particles, magnetic overlays, remnants, scenario composition, scientific mode and authentic observations.
- `ARCHITECTURE.md`: compatible integration points and data contracts for new systems.
- `GRAPHICS_TECHNIQUES.md`: candidate techniques are separated from techniques already implemented in source.
- `SCIENTIFIC_MODEL.md` and `SCIENCE_NOTES.md`: explicit scientific boundaries for the proposed additions.
- `IMPLEMENTATION_PLAN.md`: staged roadmap and dependency ordering for the current project.
- `VERIFICATION.md`: planned acceptance tests clearly separated from historical test results.
- `README.md`: clarification of current baseline versus proposed expansion.

### Status and limitations

This entry records a documentation update only. No C++ source, GLSL shader, build configuration, runtime asset, or executable was changed by this documentation pass. No new tests were run. Previously recorded build/test/benchmark results remain historical results for their recorded code state and have not been revalidated by this update.

### Direction established

Prioritise deeper cloud morphology, volumetric quality, layered stellar rendering and deterministic particle species before broad optional scenarios. Treat magnetic fields, supernova instability-inspired structures, scientific inspection, observational integration and multi-wavelength views as separate systems with explicit scientific labels. Binary/cluster/triggered scenarios and comparative evolutionary tracks are later extensions.

## 2026-11-08 — Adopted expansion docs; entry gate; Workstream A (cloud structure)

- Adopted the user's `NEW MD SET` as the authoritative documentation (all changes
  additive). Per CLAUDE.md §20 the doc set now lives at the repository root; the old
  `docs/` copies were retired through git (history keeps them). The §18 path text was
  updated to match.
- Entry gate passed with numbers identical to the 2026-11-02 record (VERIFICATION §8.1).
- **Workstream A — implemented and verified.** New `sim/CloudField`: a seeded CPU
  field with envelope / hub-filament network / core + condensations / cometary
  globules, baked to a 128³ RGBA16F texture. Particles and renderer share it. New
  scenario input `ModelConfig::cloudSeed` (UI stepper, `--seed`).
- Decision: CPU bake instead of a GPU compute bake, because it is deterministic
  across machines, unit-testable, and lets particles and future cross-sections
  sample the identical field. Cost: ~0.5 s per seed change.
- Decision: key light moved behind the cloud. Front lighting made globules look like
  bright "planets", which VISUAL_DESIGN §20.2.3 rejects.
- Workstream B (partial): coarse shadow density, central-shadow skip and shell-aware
  empty-space skipping brought the over-budget cloud back to 6.6–12.8 ms at 1080p.
- Next: rest of B (depth-aware upsampling, quality tiers), then C (stellar layers).

## 2026-11-08 — Workstream B (volume refinement, quality tiers)

- **Implemented and verified:** joint bilateral upsampling of the reduced-resolution
  volume, and four render-quality tiers (`--quality`, UI buttons). Measurements are
  in VERIFICATION §8.3; Balanced stays at 78–149 fps at 1080p, while High is below
  60 fps on the RTX 4050 in heavy stages (documented, not hidden).
- **Deferred:** temporal accumulation (P3) and multiple-scattering approximation. No
  visible failure mode currently requires them.
- Next: Workstream C (photosphere / chromosphere / corona, spots, prominences, flare).

## 2026-11-08 — Workstream C (layered stellar atmosphere)

- **Implemented and verified:** photosphere (granulation with sub-pixel fade,
  starspots, flare footpoint), chromosphere rim, structured corona, three prominence
  arches, one deterministic flare. New scene fields: `convective`, `activity`,
  `flare`, `atmosphere`. Evidence: VERIFICATION §8.4.
- Scientific decision: Sun-like activity is gated by an outer-convection-zone
  factor derived from the calculated T_eff (≈6200–7000 K transition), so massive
  hot stars show no spots or prominences. The UI approximation text was updated.
- Artistic decision: equal surface brightness for every main-sequence star, and
  stage-5 exposure lowered to 0.6, so tone mapping keeps the limb and surface
  detail visible.
- **Deferred:** CME-like ejection (optional per PROJECT_SPEC §18.5).

## 2026-11-08 — Workstream D (particle species)

- **Implemented and verified:** stellar-wind species (strength from calculated
  luminosity; slow dusty wind for giants) and episodic jet knots. VERIFICATION §8.5.
- Lesson recorded: each random coordinate needs its own independent hash.
  Correlated seeds made curve and box artifacts twice (here and in the GLUT jets).

## 2026-11-08 — Workstream F (supernova and remnant structure)

- **Implemented and verified:** shared shock/asymmetry functions, a broken shock
  rim, stratified ejecta, instability-inspired fingers, a fading persistent remnant.
  VERIFICATION §8.6, including the debugging record.
- Cost: +2–2.5 ms on remnant views (still ≥ 68 fps at 1080p Balanced).

## 2026-11-08 — Workstream E (analytic magnetic-field overlay)

- **Implemented and verified:** `sim/MagneticField` (dipole, RK4 tracer),
  `render/FieldLines` (SSBO + instanced quads), UI checkbox, `--field-lines`, and the
  mandatory model label. VERIFICATION §8.7 (includes the end-point debugging record).

## 2026-11-08 — Workstream G (scientific inspection; observation mode deferred)

- **Implemented and verified:** Scientific view with a density cross-section from the
  shared density include, viridis legend, overlays on. VERIFICATION §8.8.
- **Deferred:** observation/multi-wavelength mode. Downloading external images needs
  the user's approval and per-asset rights checks.
- **Deferred (P2):** Workstream H (binary/cluster/triggered scenarios, comparative
  laboratory). P0/P1 systems are now in place; H should start only after the user
  has reviewed this round.

## 2026-11-08 — Observation view (after the user approved downloads)

- Curated 7 authentic images linked to stages/pathways, including one genuine two-band
  pair (Pillars: Hubble visible + Webb near-infrared). Each source page and licence was
  checked first, and JSON provenance sidecars with SHA-256 checksums were written. A
  checker tool runs as a CTest test.
- New dependency: stb_image 2.30, pinned by commit and SHA-256 (JPEG decoding only).
- New `sim/ObservationCatalog` (sidecars are the single source of truth), a
  `gfx/ImageTexture` loader, a three-way view mode (Cinematic / Scientific /
  Observation) and `--observation`. Image credits were added to README §9.
- Evidence: VERIFICATION §8.11. Remaining deferred: Workstream H (scenarios,
  comparative laboratory).

## 2026-11-08 — Workstream H (partial): mass-comparison table

- Added `kComparisonMasses` (1, 2, 5, 15, 40 M☉) and a toggleable comparison table:
  the same analytic relations at each mass, the current mass highlighted, click to
  select. `--compare` flag. One new unit test. Evidence: VERIFICATION §8.12.
- Deferred, with reasons recorded: binary, cluster and triggered-compression
  scenarios, and published reference tracks.

## 2026-11-08 — Realism and energy pass (user request)

- Jets: the discrete-knot clumps (Workstream D) broke the beam into gaps. Restored a
  continuous beam; knots are now brightness only. Faster flow, hot spine, observed
  jet colours.
- Motion streaks for every moving particle family (exact earlier-time evaluation).
- Accretion bursts (sim layer, tested); compressional core heating glow; more
  realistic dust albedo, H-alpha rims, [O III] envelope; less residual fog.
- Evidence: VERIFICATION §8.13.

## 2026-11-08 — Real Milky Way sky

- ESO/S. Brunier all-sky panorama (6000×3000, CC BY 4.0) is now the background of every
  stage, mapped in galactic coordinates with an on-screen credit; `--procedural-sky`
  turns it off. sRGB texture loading was added to `ImageTexture`; the whole `assets/`
  folder is copied next to the executable. Evidence: VERIFICATION §8.14.

## 2026-11-17 — Enhancement round (user directive)

- UI: type scale (34/17/15/14.5), brighter secondary text, more opaque bordered panels,
  wider panels, section labels, accent stage bar, larger transport buttons with hover
  rings, speaker button + volume slider (M), remembered preference.
- Star: Worley granulation, per-channel quadratic limb darkening, penumbral filaments,
  faculae, gravity darkening of hot stars, giant cells on giants. WD cooling
  (`remnantTempK`, shown in the info panel); NS surface 10^6 K.
- PN ionization layers, cometary knots, AGB halo. Jets: irregular knots, paraboloid
  bow shock, fainter counter-jet.
- Sonification (sim/Sonification, audio/AudioEngine, miniaudio). Evidence: VERIFICATION §8.15.

## 2026-11-18 — Volume-first reconstruction (user directive)

- Particles demoted to an optional "Gas tracer particles" overlay. Jets, disc, streamers
  and the giant atmosphere are now ray-marched media (`inner_medium.glsl`), sub-stepped
  inside the volume loop. The layered disc geometry is drawn only with tracers.
- Real data: the Cas A 3D reconstruction (Chandra/CXC), baked to a 3D texture at build
  time, drives the neutron-star remnant. `ASSET_SOURCES.md` added at the user's request.
- Photosphere at true granule scale. Default quality tier: High. Evidence: VERIFICATION §8.16.

## 2026-11-18 — Astronomy reference library and real solar data (user directive)

- `tools/acquire_assets.py`: archive search plus resumable downloads, 75 files (~1.1 GB),
  manifest with SHA-256 and credits, download log. `tools/write_catalog.py` writes the
  catalogue and log. The library is git-ignored; the build copies only the runtime assets.
- `tools/build_observations.py`: 63 Observation-view entries. The panel now has a ←/→
  navigator and loads only the selected image.
- `tools/bake_sun_magnetogram.py`: HMI CR 2284 drives spots, plage, the corona and the
  active-region anchors. The cloud has sharper ionization-front edges.
- The ground-up rewrite the user permitted was not done. The volume-first engine from
  the previous round already matches the requested architecture, so this round put real
  data into it instead. Evidence: VERIFICATION §8.17.
