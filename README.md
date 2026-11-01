# STELLARIS

An interactive, cinematic 3D visualization of a star's life: from a molecular
cloud through gravitational collapse, protostar, accretion disc and bipolar jets,
and the main sequence, to a mass-dependent fate (white dwarf, neutron star or
black hole). It is a Computer Graphics academic project, native C++ / OpenGL 4.5,
for Windows.

The repository contains two independent programs:

| Program | What it is | Source |
|---|---|---|
| `stellaris.exe` | The main application (GLFW + OpenGL 4.5 core + Dear ImGui) | `app/` |
| `stellaris_glut_demo.exe` | A separate **freeGLUT / OpenGL** demonstration | `glut_demo/` |

---

## 1. Requirements

* Windows 10/11, a GPU with OpenGL 4.5 (developed on an RTX 4050 Laptop GPU).
* **Visual Studio 2022** or **Build Tools for Visual Studio 2022** with the
  "Desktop development with C++" workload (MSVC + Windows SDK).
* **CMake 3.24 or newer** on `PATH`.
* Internet access **once**, for the first configure step. CMake downloads these
  pinned dependencies into `build/_deps`: GLFW 3.4, GLM 1.0.1, Dear ImGui
  1.91.5 and freeGLUT 3.6.0. After that, building and running work offline.
  The OpenGL loader (glad) is already included in `third_party/glad`.

## 2. Build

Open **Command Prompt** or **PowerShell** in the project folder and run:

```bat
build.bat
```

The script configures, builds Release, and runs the unit tests. The same steps
done by hand:

```bat
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release --parallel
ctest --test-dir build -C Release --output-on-failure
```

Outputs: `build\bin\Release\stellaris.exe`, `stellaris_glut_demo.exe`,
`stellaris_tests.exe`. Shaders are copied next to the executable
(`build\bin\Release\shaders`).

### Building the GLUT demonstration on its own

```bat
cmake -S glut_demo -B glut_demo\build
cmake --build glut_demo\build --config Release
glut_demo\build\bin\Release\stellaris_glut_demo.exe
```

## 3. Run

```bat
run.bat                 :: main application
run_glut_demo.bat       :: freeGLUT demonstration
```

Useful options for `stellaris.exe`:

| Option | Effect |
|---|---|
| `--fullscreen` | start in full screen (F11 toggles) |
| `--play` | start playing immediately |
| `--time T` | start at timeline second T (0–148) |
| `--mass M` | initial stellar mass, 1–40 M☉ |
| `--rotation R` | relative core rotation, 0.6–1.4 |
| `--hide-ui` | start with the interface hidden |
| `--capture DIR T1,T2,…` | render those times to PNG files and exit |
| `--selftest` | automated playback/seek/reset/resize checks against the real renderer |
| `--benchmark` | measure frame times for every stage and exit |

## 4. Controls (main application)

| Input | Action |
|---|---|
| **Space** | play / pause (at the end, play restarts) |
| **← / →** | seek −5 s / +5 s (hold **Shift** for 1 s) |
| **1 – 6** | jump to a stage |
| **↑ / ↓** | playback speed (0.25×, 0.5×, 1×, 2×, 4×) |
| **R** or **Home** | reset: time 0, paused, 1×, camera recentered |
| Click / drag the **timeline** | seek; click a **stage name** to select that stage |
| **Left-drag** in the scene | orbit the camera (an offset on top of the authored camera) |
| **Mouse wheel** | zoom |
| **C** or double-click | recenter the camera on the authored path |
| **H** | hide / show the interface |
| **F11** | full screen;  **Esc** leaves full screen |
| **F12** | screenshot to `screenshots\` |
| **F5** | reload shaders (development) |

Scientific controls are in the right-hand panel. **Initial stellar mass** changes
the main-sequence star's color, size and luminosity, and selects the final
pathway. **Initial core rotation** changes the angular momentum, and with it the
size of the accretion disc. **3D labels** toggles the in-scene annotations.

GLUT demonstration: **Space** pause, **N/P** next/previous phase, **1–6** jump,
**+/−** speed, **R** reset, **W** wireframe, **G** fog, **F** full screen,
**H** help, mouse drag to orbit, wheel to zoom, **right-click** for the menu,
**Esc** to quit.

## 5. Documentation

| Document | Contents |
|---|---|
| `docs/ARCHITECTURE.md` | technology choice, module layout, frame pipeline, timeline model |
| `docs/GRAPHICS_TECHNIQUES.md` | the computer graphics techniques actually implemented |
| `docs/SCIENCE_NOTES.md` | scientific model, equations, sources, approximations, limitations |
| `docs/VERIFICATION.md` | tests, measurements, visual inspection results, known limitations |
| `docs/DEVELOPMENT_LOG.md` | decisions and milestone history |
| `PROJECT_SPEC.md`, `VISUAL_DESIGN.md`, `SCIENTIFIC_MODEL.md`, `IMPLEMENTATION_PLAN.md` | the original requirements |

## 6. Troubleshooting

* **"Could not create an OpenGL 4.5 core window"**: update the GPU driver. On
  hybrid-graphics laptops the program asks for the discrete GPU automatically.
* **Black window or shader errors**: check `build\bin\Release\stellaris.log`. The
  `shaders` folder must sit next to `stellaris.exe` (the build copies it).
* **Configure fails while downloading**: the first configure needs internet
  access. Run it again once you are connected.
