# Computer Graphics techniques implemented

Only techniques that exist in the code are listed. Each line names the file
that implements it.

## Main application (OpenGL 4.5 core)

| Technique | Where | Notes |
|---|---|---|
| Compute shader texture synthesis | `shaders/noise3d.comp` | 128³ RGBA16F tileable noise volume (gradient fBm, Worley fBm, ridged fBm) generated on the GPU at startup with integer hashing |
| Volumetric ray marching | `shaders/volume.frag` | up to 96 fixed-length steps (72 before 2026-11-08) with per-pixel interleaved-gradient jitter; early exit at 1 % transmittance |
| Single scattering with shadow rays | `volume.frag` | two lights: a distant external field and the central star; 4-sample optical-depth march toward each |
| Henyey–Greenstein phase function | `volume.frag` | forward scattering (g = 0.35 / 0.45) |
| Wavelength-dependent extinction | `volume.frag` | dust attenuates blue more than red (reddening) |
| Energy-conserving step integration | `volume.frag` | analytic integration of in-scattering over each step (S − S·T)/σ |
| Domain warping | `volume.frag` | low-frequency noise displaces the sampling point (filamentary structure) |
| Procedural deformation of a density field | `volume.frag` | collapse mapping r₀ = r·h(r) with a Jacobian density gain, swirl and flattening |
| Reduced-resolution rendering + upsampled composite | `Renderer.cpp`, `composite.frag` | volume at ½ resolution, premultiplied "over" blend |
| Depth-aware ray termination | `volume.frag` | rays stop at the star using the full-resolution depth buffer |
| Deterministic GPU particles (stateless) | `shaders/particles.vert` | 276k particles (290k with the stellar wind since 2026-11-08); positions computed in closed form from seed and time (free-fall trajectory, Keplerian orbit, jets, ejecta) |
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

### Expansion techniques added 2026-11-08 — implemented and verified

| Technique | Where | Notes |
|---|---|---|
| Seeded multi-component cloud structure field | `sim/CloudField.*` | CPU-evaluated envelope (blob union + value-noise modulation + voids), hub-filament network (Bézier centerlines, Plummer-like transverse profile, branches), dense core + condensations, cometary globules; baked multi-threaded to a 128³ grid (~0.45 s) and uploaded as an RGBA16F 3D texture (16 MiB). Particles sample the same grid. |
| Per-component density treatment | `volume.frag` (`baseDensity`) | envelope strongly eroded by noise, filaments textured, core/globules compact |
| Coarse density for shadow rays | `volume.frag` (`densityCoarse`) | structure field without fine noise for the 8 light samples per step; rendering approximation |
| Empty-space skipping (shell-aware) | `volume.frag` | 2× step where the structure is empty and no ejecta shell is near; fixed step inside the cloud |
| Joint bilateral (depth-aware) upsampling | `composite.frag` | each of the 4 nearest low-res volume texels is weighted by bilinear weight × similarity of the scene depth it was rendered with to this pixel's depth: no cloud halo across the stellar limb |
| Render quality tiers | `render/RenderQuality.h`, `Renderer::setQuality` | Performance / Balanced / High / Capture change volume resolution (⅓, ½, ¾, full), ray steps (64–160) and particle subsampling (stable subset, brightness-compensated); scene state never changes |
| Layered photosphere | `star.frag`, `stellar_activity.glsl` | limb darkening; two-scale granulation-like pattern that fades below ~2–6 px per cell (no aliasing); umbra/penumbra starspots with noise-perturbed edges at fixed positions in the rotating stellar frame; flare footpoint kernel |
| Chromosphere + structured corona | `corona.frag` (billboard, depth-tested) | thin reddish limb rim; smooth halo plus world-anchored radial streamers concentrated toward the stellar equator; flare brightening above the active region |
| Prominence ribbons | `prominence.vert/.frag` | 3 instanced camera-facing triangle-strip arches anchored on active regions, depth-tested against the star; emission suppressed where seen against the disc (where real prominences appear dark) |
| Typed particle species | `particles.vert`, `ParticleSystem` | five deterministic families (ambient dust, collapse infall → Keplerian disc, jet, final-stage ejecta, stellar wind), 290k particles. Jets: episodic knots sharing a launch phase (Gaussian-shaped clumps) inside a softer continuous outflow. Wind: radial outflow at constant presentation speed with mild streaming; hot-star or dusty-giant coloring |
| Structured supernova remnant | `common.glsl` (`snShockRadius`, `snAsymmetry`), `particles.vert`, `volume.frag` | shared decelerating shock-radius law and fixed dipole + quadrupole asymmetry; volume: thin shock front with a sharp outer edge, broken into patches by a thresholded two-scale fBm mask, plus faint interior emission; particles: three stratified ejecta layers and radial "fingers" along 48 seeded directions (30 % of the ejecta) |
| Back-lit external illumination | `CloudField::keyLightDirection`, `volume.frag` | light from behind the cloud: rim lighting, globules seen in silhouette |

Measured cost and visual evidence: `VERIFICATION.md` §8.

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

---

## 3. Candidate techniques for the next STELLARIS expansion

**Important status rule:** the table above is the record of techniques implemented in the current source as previously inspected. The techniques below are candidates, not claims that they already exist. Promote a technique into the implemented table only after the actual code path is present and tested. Do not add a technique solely to increase the technique count.

### 3.1 Volume-rendering improvements

| Candidate | Purpose | Engineering constraints | Acceptance evidence |
|---|---|---|---|
| Adaptive/density-aware ray marching | Spend samples where density or optical depth changes rapidly | Bounded step sizes; stable integration; avoid thin-feature skipping | Step-count/GPU-time comparison plus fixed-camera images showing no banding or lost filaments |
| Depth-aware/bilateral upsampling | Improve half-resolution volume edges against full-resolution geometry | Correct depth sampling and premultiplied-alpha handling | No halos at stellar limb/opaque silhouettes; compare to full-resolution reference capture |
| Multi-scale density composition | Preserve envelope, filaments, cores and fine detail at different scales | Stable seed and sampling footprint; avoid high-frequency aliasing | Wide/medium/close views and timeline-seek consistency |
| Ambient/multiple-scattering approximation | Reduce unnaturally black dense regions and improve soft volumetric fill | Clearly approximate; bounded energy; do not make volume self-luminous | Exposure sweep, energy/brightness review and GPU measurement |
| Temporal accumulation/reprojection (experimental) | Reduce noise in temporally stable volume samples | History invalidation on seek/camera/settings/resize/scenario changes; deterministic capture path | No ghost trails or time-dependent screenshots; compare static and moving camera tests |
| Improved light-space optical-depth sampling | Better volumetric shadow definition | Sample count is GPU cost; avoid bias and excessive softness | Side-by-side shadow images and pass timing |

### 3.2 Procedural geometry and structured field rendering

| Candidate | Purpose | Preferred implementation direction | Constraints |
|---|---|---|---|
| Filament density ridges | Give molecular clouds a coherent large-scale structure | Deterministic procedural masks/curves feeding the volume density | Must remain volumetric; do not simply overlay glowing tubes |
| Dense-core/globule masks | Create readable condensations and obscuring regions | Seeded analytic/procedural density components | Separate visual density from physical density unless calibrated |
| Field-line curves | Explain analytic magnetic geometry | Deterministic vector-field integration, then render as thin ribbons/segments | Label analytical/prescribed; bounded number and length |
| Prominence/loop curves | Show localised stellar atmospheric structures | Authored/parametric curves with emissive ribbon rendering | Anchored to the stellar surface; no random unattached loops |
| Supernova shell perturbations | Create asymmetric ejecta morphology | Seeded low-/mid-frequency surface displacement and clump masks | Do not describe as a fluid instability solver |
| Cross-section plane | Inspect the existing density/temperature field | Clip/slice surface sampling the same shader/model fields | Units and normalization must be explicit |

### 3.3 Stellar-emission and atmospheric shading

| Candidate | Role | Notes |
|---|---|---|
| Photosphere refinement | Granulation-like pattern, spots, limb darkening | Emissive/blackbody-informed rendering; not a conventional albedo-driven PBR sphere |
| Chromosphere layer | Thin atmospheric emission | Must have distinct optical/opacity contribution and avoid a thick shell silhouette |
| Structured corona | Low-density outer emission and streamers | Use directional/asymmetric structures; bloom is not a replacement for geometry/shading |
| Localised flare event | Surface-originating time-bounded emission | Deterministic event window and restrained HDR response |
| CME-like ejecta | Outward shell/particles from a selected surface region | Prescribed visual model unless a plasma solver is genuinely implemented |
| Accretion-disc thermal shading refinement | Better radial and angular structure | Keep the existing temperature profile's illustrative status clear; do not imply full radiative transfer |

### 3.4 Particle rendering and simulation options

| Candidate | Benefit | Default recommendation |
|---|---|---|
| Species-specific trajectory functions | Clear difference between infall, orbit, jet, wind and ejecta | **Preferred**: retain deterministic seed/time evaluation |
| Velocity-aligned streaks/ribbons | Communicate flow direction at high speed | Use selectively; suppress when they clutter the image |
| Compute-shader-updated SSBO particles | Supports interactions that cannot be expressed as closed-form time functions | **Experimental**: only after measuring a real limitation in the current stateless design |
| GPU prefix-sum/culling for large particle counts | Reduce overdraw/work for very large effects | Defer until a profiler identifies particle submission/culling as a bottleneck |
| Soft particle depth fade | Reduce hard intersections with volumes/geometry | Use only when depth and compositing semantics remain correct |

### 3.5 Observation and scientific-visualisation techniques

| Candidate | Purpose | Data rule |
|---|---|---|
| Multi-band image comparison | Explain wavelength-dependent observations | Use correctly identified source products; do not tint a single band and call it another wavelength |
| Image-plane/projection presentation | Show real observations in context | Preserve credits, scale/framing and source metadata; state that the image is 2D |
| Density/temperature colour mapping | Inspect the underlying procedural/model field | Label normalized/prescribed values; use perceptually ordered colour maps and explicit ranges |
| Velocity-vector overlay | Show direction and magnitude where defined | Render only from an actual model velocity field; never infer vectors from visual particle motion |
| Magnetic-field overlay | Show a defined vector field | State field model and units/normalization; no MHD claim for analytic fields |
| Comparative evolutionary curves | Compare representative masses or published tracks | Keep reference datasets and simplified formula outputs clearly distinguished |

### 3.6 Techniques deliberately not required by default

Do not add these simply because they appear in advanced graphics articles or tutorials:

- **Geometry/tessellation shaders:** not inherently better than a vertex/compute pipeline; use only for a concrete geometry-generation need.
- **Screen-space reflections:** most cloud/star scenes contain few meaningful reflective surfaces; likely low value compared with volume quality and emission control.
- **Depth of field and motion blur:** optional cinematographic effects only; they can obscure scientific detail and destabilise screenshots.
- **Chromatic aberration:** cosmetic and generally inappropriate for a scientific presentation unless a specific optical model requires it.
- **Full PBR material stack on stars:** stars are luminous plasma bodies; emissive/radiative shading is the appropriate conceptual model.
- **Unbounded temporal denoising:** conflicts with seeking/deterministic capture unless history invalidation is rigorous.
- **Path tracing or full radiative transfer:** disproportionate to the current real-time Computer Graphics scope.
- **A general-purpose render graph:** unnecessary until pass complexity demonstrably justifies it.

### 3.7 Required reporting for every adopted technique

For each new technique, record in this existing file or the relevant implementation/design file:

1. the visual/scientific problem it solves;
2. the selected algorithm and why it fits OpenGL 4.5;
3. shader/program/buffer/render-target ownership;
4. input parameters and units/normalization;
5. time/seed determinism behaviour;
6. known artifacts and fallback path;
7. GPU-time and VRAM cost on the reference machine;
8. visual captures and test result;
9. whether it is physically derived, an approximation, or purely artistic.
