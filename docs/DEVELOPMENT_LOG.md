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

### Claude Code setup (this session) — see docs/CLAUDE_SETUP.md

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
