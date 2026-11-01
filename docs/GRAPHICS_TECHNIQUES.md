# Computer Graphics techniques implemented

Only techniques that exist in the code are listed. Each line names the file
that implements it.

## Main application (OpenGL 4.5 core)

| Technique | Where | Notes |
|---|---|---|
| Compute shader texture synthesis | `shaders/noise3d.comp` | 128³ RGBA16F tileable noise volume (gradient fBm, Worley fBm, ridged fBm) generated on the GPU at startup with integer hashing |
| Volumetric ray marching | `shaders/volume.frag` | 72 fixed-length steps with per-pixel interleaved-gradient jitter; early exit at 1 % transmittance |
| Single scattering with shadow rays | `volume.frag` | two lights: a distant external field and the central star; 4-sample optical-depth march toward each |
| Henyey–Greenstein phase function | `volume.frag` | forward scattering (g = 0.35 / 0.45) |
| Wavelength-dependent extinction | `volume.frag` | dust attenuates blue more than red (reddening) |
| Energy-conserving step integration | `volume.frag` | analytic integration of in-scattering over each step (S − S·T)/σ |
| Domain warping | `volume.frag` | low-frequency noise displaces the sampling point (filamentary structure) |
| Procedural deformation of a density field | `volume.frag` | collapse mapping r₀ = r·h(r) with a Jacobian density gain, swirl and flattening |
| Reduced-resolution rendering + upsampled composite | `Renderer.cpp`, `composite.frag` | volume at ½ resolution, premultiplied "over" blend |
| Depth-aware ray termination | `volume.frag` | rays stop at the star using the full-resolution depth buffer |
| Deterministic GPU particles (stateless) | `shaders/particles.vert` | 276k particles; positions computed in closed form from seed and time (free-fall trajectory, Keplerian orbit, jets, ejecta) |
| Point sprites with size attenuation and coverage fading | `particles.vert/.frag` | sub-pixel particles fade by covered area (no flicker); near-camera fade |
| Procedural star field | `shaders/sky.frag` | hashed cells on the view sphere; blackbody star colors; pixel-footprint anti-aliasing |
| Procedural surface shading | `shaders/star.frag` | animated cellular pattern, linear limb darkening I(μ)=1−u(1−μ) |
| Procedural polar-grid mesh + instancing | `gfx/Geometry.cpp`, `disc.vert` | flared disc drawn as 5 instanced layers |
| Differential-rotation texture advection | `disc.frag` | noise sampled at φ − ω(r)·t with ω ∝ r^−1.5 |
| Blackbody color lookup texture | `Renderer.cpp`, `common.glsl` | 1D LUT built on the CPU from Planck × CIE color-matching functions |
| Camera-facing billboards | `glow.vert/.frag` | halos, supernova flash with ray structure |
| HDR rendering | `Renderer.cpp` | RGBA16F scene target, R11G11B10F bloom chain |
| Physically-based bloom | `bloom_down.frag`, `bloom_up.frag` | 13-tap downsample (Karis average on first level), 3×3 tent upsample, 6 levels |
| Filmic tone mapping | `final.frag` | ACES fitted curve, exposure from the timeline, vignette, sRGB encoding, triangular dither |
| Screen-space gravitational lensing | `lens.frag` | thin-lens point-mass mapping β = θ − θE²/θ; shadow disc; photon ring |
| Off-axis (lens-shift) projection | `Renderer.cpp` | composes the subject clear of the information panel without perspective distortion |
| Authored camera animation | `sim/CameraPath.cpp` | Catmull-Rom splines on orbit parameters, log-space distance interpolation |
| World → screen projection for annotations | `Renderer::projectToScreen`, `ui/Ui.cpp` | labels with leader lines anchored to 3D points |
| Uniform buffer objects | `common.glsl`, `Renderer.cpp` | std140 per-frame block shared by all programs |
| Direct state access, debug output, timer queries | `gfx/`, `Renderer.cpp` | GL 4.5 DSA, `GL_KHR_debug`, `GL_TIME_ELAPSED` |
| Shader preprocessing and hot reload | `gfx/Shader.cpp` | `#include` expansion; F5 reloads and keeps the old program on error |

## GLUT demonstration (`glut_demo/src/main.cpp`, fixed-function OpenGL)

| Technique | Notes |
|---|---|
| GLUT context and window | `glutInitDisplayMode(GLUT_DOUBLE \| GLUT_RGBA \| GLUT_DEPTH \| GLUT_MULTISAMPLE)` |
| GLUT callbacks | display, reshape, keyboard, special keys, mouse, motion, mouse wheel, timer (60 Hz), close |
| GLUT menus | right-click menu with a "Jump to phase" submenu |
| GLUT bitmap text | HUD with `glutBitmapString` in an orthographic overlay |
| GLUT primitives | `glutSolidSphere`, `glutWireSphere` |
| Transformations and matrix stack | `glPushMatrix`/`glPopMatrix`, rotate, scale (bipolar nebula), `gluPerspective`, `gluLookAt`, `gluOrtho2D` |
| Fixed-function lighting | positional `GL_LIGHT0` at the star; an orbiting planet (phase 5) shows the diffuse day/night terminator and a specular highlight; stars use material emission; `GL_NORMALIZE` |
| Display lists | the static cloud is compiled once with `glNewList` |
| Blending | additive (glows, jets, disc) and alpha blending; depth-mask control |
| Billboarding | glow discs built by removing rotation from the modelview matrix |
| Fog | `GL_EXP2` fog as a depth cue (toggle G) |
| Wireframe mode | `glPolygonMode` (toggle W) |
| Deterministic animation | everything derives from one time value; pause, phase jump and reset are exact |
