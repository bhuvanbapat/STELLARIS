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
