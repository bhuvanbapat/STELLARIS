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
| `--seed N` | procedural cloud morphology seed (default 1) |
| `--field-lines` | start with the analytic dipole field-line overlay on |
| `--compare` | start with the mass-comparison table open |
| `--procedural-sky` | use the procedural star field instead of the ESO Milky Way photograph |
| `--observation` | start in the Observation view (authentic credited image beside the scene) |
| `--scientific` | start in the Scientific view (density cross-section, field lines, labels) |
| `--quality Q` | `performance`, `balanced` (default), `high`, `capture`: rendering cost only |
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
size of the accretion disc. **Cinematic / Scientific / Observation** switches the view. Scientific adds a normalized density
cross-section through the core (same field as the volume) plus field lines and labels.
Observation shows an authentic, credited astronomical image for the current stage beside
the scene (with a band toggle where two genuine bands exist).
**Magnetic field (analytic dipole)** shows normalized dipole field lines around the
main-sequence star and the neutron star (a visualization, not MHD).
**Render quality** (Perf / Bala / High / Capt) changes rendering cost only.
**Cloud structure < seed >** picks another procedural cloud: same science, different
morphology. **3D labels** toggles the in-scene annotations.

GLUT demonstration: **Space** pause, **N/P** next/previous phase, **1–6** jump,
**+/−** speed, **R** reset, **W** wireframe, **G** fog, **F** full screen,
**H** help, mouse drag to orbit, wheel to zoom, **right-click** for the menu,
**Esc** to quit.

## 5. Documentation

| Document | Contents |
|---|---|
| `ARCHITECTURE.md` | technology choice, module layout, frame pipeline, timeline model |
| `GRAPHICS_TECHNIQUES.md` | the computer graphics techniques actually implemented |
| `SCIENCE_NOTES.md` | scientific model, equations, sources, approximations, limitations |
| `VERIFICATION.md` | tests, measurements, visual inspection results, known limitations |
| `DEVELOPMENT_LOG.md` | decisions and milestone history |
| `PROJECT_SPEC.md`, `VISUAL_DESIGN.md`, `SCIENTIFIC_MODEL.md`, `IMPLEMENTATION_PLAN.md` | the original requirements |

## 6. Troubleshooting

* **"Could not create an OpenGL 4.5 core window"**: update the GPU driver. On
  hybrid-graphics laptops the program asks for the discrete GPU automatically.
* **Black window or shader errors**: check `build\bin\Release\stellaris.log`. The
  `shaders` folder must sit next to `stellaris.exe` (the build copies it).
* **Configure fails while downloading**: the first configure needs internet
  access. Run it again once you are connected.

## 7. Current baseline versus proposed expansion

The existing feature descriptions and verification record refer to the implemented baseline documented in the tables above and in `GRAPHICS_TECHNIQUES.md`, `SCIENCE_NOTES.md`, and `VERIFICATION.md`. The expanded molecular-cloud environment, layered stellar atmosphere, typed particle phenomena, analytic magnetic-field overlays, structured supernova morphology, scientific inspection mode, observation comparison, and optional formation scenarios are **roadmap items unless the current source and verification record explicitly show them as completed**.

The default product remains the six-stage single-star visualization. Expansion work must preserve its timeline, deterministic seeking, mass-dependent outcomes, native OpenGL architecture and separate freeGLUT demonstration.

### Expansion direction

The next development direction is to deepen the existing systems rather than create unrelated scenes:

- give the molecular cloud a hierarchy of envelope, filaments, dense cores, dust lanes and cavities;
- refine volume sampling, lighting and upsampling without sacrificing deterministic playback;
- build distinct photosphere/chromosphere/corona visual layers and selected surface activity;
- differentiate infall, disc, jet, wind and ejecta particle motion;
- add optional analytic magnetic-field visualisation and instability-inspired remnant structure;
- add scientific inspection and carefully attributed astronomical observations;
- add optional binary/cluster/triggered-formation presets and mass comparisons only after the shared systems are stable.

These are not claims that the application solves full hydrodynamics, MHD, stellar interiors, nuclear networks, N-body dynamics or wavelength-resolved radiative transfer.

## 8. Expanded documentation map

| Document | Authority |
|---|---|
| `CLAUDE.md` | Agent operating rules, source-of-truth order, expansion constraints and completion discipline |
| `PROJECT_SPEC.md` | Functional scope, priorities and acceptance criteria |
| `VISUAL_DESIGN.md` | Art direction, scene composition, material/volume goals and visual acceptance |
| `SCIENTIFIC_MODEL.md` | Physical concepts, equations, units, assumptions and scientific boundaries |
| `SCIENCE_NOTES.md` | What the current application actually calculates, prescribes and draws; proposed extensions are explicitly separated |
| `ARCHITECTURE.md` | Current architecture and compatible subsystem integration points |
| `GRAPHICS_TECHNIQUES.md` | Techniques verified in source versus candidate techniques not yet implemented |
| `IMPLEMENTATION_PLAN.md` | Existing implementation sequence and controlled expansion roadmap |
| `VERIFICATION.md` | Historical tests/results and separate planned tests for future features |
| `DEVELOPMENT_LOG.md` | Chronological decisions, progress and verification status |
| `CLAUDE_SETUP.md` | Tooling and Claude Code setup notes from the recorded audit |

Do not treat roadmap items as current functionality. For current build instructions and controls, use the earlier sections of this README and validate against the current executable.

## 9. Image credits (Observation view)

The Observation view shows these authentic images unmodified, each with its full credit
line beside it. Full provenance (source page, instrument, band, processing, licence,
SHA-256) is in `assets/observations/*.json`; `tools/check_observations.py` validates it
and runs as a CTest test.

| Image | Credit | Licence |
|---|---|---|
| Pillars of Creation, visible (heic1501a) | NASA, ESA/Hubble and the Hubble Heritage Team | CC BY 4.0 |
| Pillars of Creation, NIRCam (weic2216a) | NASA, ESA, CSA, STScI; J. DePasquale, A. Koekemoer, A. Pagan (STScI) | CC BY 4.0 |
| HH 211, NIRCam (weic2322a) | ESA/Webb, NASA, CSA, T. Ray (Dublin Institute for Advanced Studies) | CC BY 4.0 |
| Solar limb prominence, SDO/AIA 304 Å (SVS 4277) | NASA's Scientific Visualization Studio, the SDO Science Team, and the Virtual Solar Observatory | Public domain |
| Southern Ring Nebula, NIRCam + MIRI (weic2207a) | NASA, ESA, CSA, STScI, and the Webb ERO Production Team | CC BY 4.0 |
| Crab Nebula (heic0515a) | NASA, ESA and Allison Loll/Jeff Hester (Arizona State University). Acknowledgement: Davide De Martin | CC BY 4.0 |
| M87* black hole (eso1907a) | EHT Collaboration | CC BY 4.0 |
| Milky Way panorama, background sky of every stage (eso0932a, `assets/sky/`) | ESO/S. Brunier | CC BY 4.0 |

Third-party code: stb_image 2.30 (Sean Barrett), MIT / public domain.
