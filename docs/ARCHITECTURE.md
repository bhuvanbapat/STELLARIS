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
3. **Volume**: ray-marched molecular cloud plus nebula/remnant shell emission, at
   half resolution (RGBA16F, premultiplied), then composited over the HDR target.
4. **Transparent structures**: accretion disc (5 instanced layers, premultiplied
   alpha), 276,000 deterministic GPU particles (additive point sprites), pulsar
   beams, halos and the supernova flash (additive).
5. **Lensing** (black-hole pathway only): screen-space point-mass deflection.
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
