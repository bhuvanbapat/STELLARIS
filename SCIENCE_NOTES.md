# STELLARIS — Scientific model as implemented

This file records what the application actually computes, what it only
prescribes or draws artistically, and the sources behind its numbers. It
implements the requirements of `SCIENTIFIC_MODEL.md`.

**Provenance tags used in the UI quantities table:** `calculated` (output of
an implemented equation), `parameter` (user input), `prescribed` (an animation
variable with no physical units), `model relation` (a scaling law the
visualization follows), `reference` (a literature value shown for context),
`illustrative` (an artistic or representative choice), `category` (an
illustrative classification).

## 1. Timeline

Six stages over 148 presentation seconds. The timeline is explicitly labeled
"not to physical time scale". The stages overlap in reality; the visualization
separates them for explanation.

## 2. Calculated quantities

| Quantity | Equation | Source | Limitations |
|---|---|---|---|
| Main-sequence luminosity L | L/L☉ = (M/M☉)⁴ for 0.43–2 M☉; 1.4 (M/M☉)^3.5 for 2–55 M☉; 32000 (M/M☉) above 55 M☉ | Salaris & Cassisi (2005); Duric (2004), as tabulated in Wikipedia "Mass–luminosity relation" | accurate only to factors of order unity; ignores composition and age |
| Main-sequence radius R | R/R☉ = 1.05 M^0.94 (M < 1.66), 1.37 M^0.54 (M ≥ 1.66) | Demircan & Kahraman (1991), as reported by Camposeo (2025, arXiv:2508.08845) | empirical; ~6 % discontinuity at the break; gives 1.05 R☉ for the Sun |
| Effective temperature | T = T☉ (L/R²)^¼, from L = 4πR²σT⁴ | Stefan–Boltzmann law; T☉ = 5772 K (IAU 2015 Resolution B3) | inherits the errors of L and R (Sun-like star → ≈5630 K) |
| Main-sequence lifetime | t ≈ 10¹⁰ yr · (M/M☉)/(L/L☉) | standard fuel/burn-rate scaling | order of magnitude only; overestimates the most massive stars' lifetimes. Shown as "~" with an "order of mag." tag |
| Free-fall time | t_ff = √(3π / 32Gρ), ρ = n(H₂)·2.8 m_H with illustrative n(H₂) = 10⁴ cm⁻³ → ≈3×10⁵ yr | pressureless uniform-sphere collapse; G (CODATA 2018) | real cores are not uniform or pressureless; the density is a representative value |
| Star color | Planck spectrum × CIE 1931 color-matching functions (multi-lobe Gaussian fit) → XYZ → linear sRGB, normalized | Wyman, Sloan & Shirley (2013), JCGT 2(2) | a display color, not a measured color; brightness is compressed artistically |

Selectable mass range: 1–40 M☉. Lower-mass stars live longer than about
10¹⁰ yr and none has yet completed the final stage.

## 3. Prescribed / model-relation behavior (not simulations)

- **Collapse (particles):** each core particle follows the pressureless
  free-fall solution r = r₀cos²β, t = t_ff(2/π)(β + sinβcosβ). Its t_ff grows
  with starting radius (inside-out collapse, as for a centrally concentrated
  core). Angular momentum is conserved during infall (φ̇ = j/r²), so material
  spins up. Each particle lands at a centrifugal radius r_c ∝ j², where j comes
  from the initial rotation (the "core rotation" control scales r_c ∝ Ω²).
  Material with r_c inside the star is accreted. After landing it follows
  Keplerian rotation ω ∝ r^−1.5 with an exponential inward drift (a
  prescribed stand-in for viscous accretion). No pressure, magnetic fields,
  turbulence, self-gravity or hydrodynamics are computed.
- **Cloud structure (since 2026-11-08):** a seeded procedural field with separately
  identifiable components: an irregular envelope with voids, a hub-filament network
  converging on the dense core (filaments have a Plummer-like transverse profile),
  secondary condensations, and cometary globules whose tails point away from the
  external light. All values are **normalized, dimensionless** visual densities: a
  procedural density structure, not a turbulent-cloud or self-gravity solution. The
  UI says "Procedural hub-filament cloud (illustrative, not a measured region)". The
  hub-filament arrangement and cometary globule shapes are inspired by observed
  morphology; no specific region is reproduced.
- **Stellar atmosphere and activity (since 2026-11-08):** the main-sequence star has
  a photosphere (blackbody color of the calculated T_eff, linear limb darkening
  u = 0.6, granulation-like pattern), a thin chromospheric rim, a corona and, only
  for stars with an outer convection zone, starspots, three prominence loops and one
  flare event (t ≈ 91–95 s). The convection-zone factor is derived from the
  calculated T_eff: 1 below 6200 K, 0 above 7000 K, smooth in between. This
  reflects that Sun-like magnetic activity needs an outer convection zone, which
  hotter, more massive stars lack; the transition is approximate. Spot layout,
  loop shapes, flare timing and corona streamers are **parametric visual
  representations**: no convection, flux emergence, reconnection or plasma
  transport is simulated, and no flare energy is implied. Every main-sequence star
  is drawn with the same surface brightness (artistic). Luminosity differences are
  shown through size and color.
- **Stellar wind (since 2026-11-08):** a particle species showing outflow from the
  star. On the main sequence its visual strength is clamp(log10(L/L☉)/5.5, 0.12, 1)
  of the calculated luminosity: weak for the Sun, strong for massive stars,
  reflecting that hot massive stars drive far stronger winds. During the giant
  phase it becomes a slow, dusty mass-loss wind. Speeds and densities are
  presentation values, not mass-loss rates.
- **Jets (revised 2026-11-08, realism pass):** a continuous, collimated beam with a
  hot, narrow spine. Internal shocks ("knots", as in Herbig–Haro jets) are travelling
  brightness enhancements, not separate clumps; the discrete-clump version broke the
  beam into gaps and was removed at the user's request. The flow is ~3× faster than
  before, to reflect that jets (~100–500 km/s) far outpace the infalling gas.
  Colours follow observed jets: bluish-white near the source, H-alpha/[S II] rose
  along the beam, a red-orange bow shock. Presentation speeds, not physical ones.
- **Accretion bursts (2026-11-08):** three prescribed outbursts (t = 47, 61, 73.5 s;
  fast rise, ~3 s decay) brighten the protostar and its embedded glow, after
  FU Orionis / EX Lupi-type episodic accretion. Timing and amplitude illustrative.
- **Cloud appearance (2026-11-08):** dust albedo lowered to ~0.5 (typical of
  interstellar grains in the visible); H-alpha emission on cloud surfaces facing the
  external stars (photo-ionised rims, as on the Pillars of Creation); faint [O III]
  teal in thin envelope gas; a compressional-heating glow in the dense core that
  grows with collapse (artistic). The residual cloud after dispersal is reduced from
  18% to 8%.
- **Supernova remnant structure (since 2026-11-08):** a forward shock front (with
  a decelerating, presentation-timed radius), stratified ejecta behind it (inner
  teal "oxygen-like", middle red "hydrogen-alpha-like", outer blue-white layers;
  colors artistic), fixed large-scale asymmetry, clumps, and radial fingers. The
  fingers are **instability-inspired procedural morphology**, not a solved
  Rayleigh–Taylor flow or a hydrodynamic supernova. The white-dwarf pathway still
  never shows a supernova (unit-tested).
- **Magnetic field lines (since 2026-11-08, optional overlay, off by default):** an
  ideal dipole B = (1/r³)[3(m·r̂)r̂ − m] with μ₀/4π dropped (**normalized**; unit
  moment, unit stellar radius). Lines are RK4 integral curves of B/|B| (step 0.02 R;
  closed lines end at the bisected surface crossing; open lines are cut at 5 R, or
  45 R for the neutron star; at most 2000–4000 steps). Main-sequence star: dipole
  along the rotation axis. Neutron star: dipole tilted 35° and rotating with the
  pulsar beams. The on-screen label reads "ANALYTIC DIPOLE FIELD — VISUALIZATION ·
  normalized geometry, not MHD". The lines are field geometry, not particle paths;
  nothing couples them to the jets, activity or evolution.
- **Scientific view (since 2026-11-08):** a Cinematic/Scientific switch. The
  Scientific view adds a density cross-section and turns on the field lines and
  labels. The cross-section shows the **normalized procedural density ρₙ
  (dimensionless)** of the rendered field (including the prescribed collapse
  mapping) on a plane through the core facing the camera. The legend runs 0–4
  and shows "≥ 4" where it saturates. No physical units are attached; a
  temperature or velocity overlay is not provided, because the model owns no such
  fields beyond the prescribed trajectories.
- **Collapse (volume):** a smooth prescribed mapping draws the cloud's inner
  density inward, with a Jacobian density gain, swirl and flattening. It is
  illustrative only.
- **Disc temperature:** T ∝ r^−3/4, the steady thin-disc profile, anchored at an
  illustrative 3800 K at the inner edge. It is used for color only.
- **Protostar:** drawn larger than its main-sequence star and contracting
  toward it (pre-main-sequence contraction). Its color temperature
  (2400 → 4000 K) is illustrative.
- **Jets:** a procedural particle stream with outward-moving knots and a bow
  shock, carrying a small share of the particles. No MHD launching is modeled,
  and the text states that most material is accreted.
- **Envelope clearing:** outflow cavities and stellar feedback thin the
  remaining cloud (prescribed).

## 4. Mass-dependent final stage (illustrative categories)

| Initial mass | Pathway shown | Remnant |
|---|---|---|
| < 8 M☉ | red giant → envelope shed as planetary-nebula shells → hot core exposed (nebula glows once ionized) | white dwarf, **no supernova** |
| 8 – 25 M☉ | red supergiant → core collapse → supernova → expanding remnant | neutron star (beams shown; the text says only some are seen as pulsars) |
| ≥ 25 M☉ | red supergiant → collapse with a weaker explosion (the text says it may be faint or absent) | black hole: dark shadow and point-mass lensing |

**Qualifications, also shown in the app:** the ~8 M☉ boundary varies with
metallicity and mass loss in the literature (roughly 6–12 M☉). The NS/BH
outcome is not monotonic in mass ("islands of explodability", Sukhbold et al.
2016) and depends on rotation, mass loss and binary interaction. Masses within
6–10 M☉ or 18–30 M☉ trigger an "uncertain boundary" notice. Remnants are
enormously enlarged and labeled "(enlarged)". The pulsar rotation is slowed for
visibility. The lensing is a simplified thin-lens point-mass mapping with
symbolic sizes.

## 5. Artistic effects (no physical claim)

Cloud colors (indigo/violet with a dusty-magenta emission tint), the external
light direction, star surface cells (not simulated convection), halos, bloom,
nebula emission colors (teal inner / red outer, inspired by ionized oxygen and
hydrogen-alpha emission), supernova ray structure, exposure changes, and all
on-screen sizes and distances (strongly compressed: star radius ∝ R^0.35).

## 6. Representative explanatory facts in the UI

- Molecular clouds are mostly molecular hydrogen with some dust, at roughly
  10–20 K (tagged "reference").
- Only a small fraction of a cloud's mass becomes stars; not every cloud
  collapses.
- A protostar shines from contraction and accretion, not sustained fusion.
- A main-sequence star is powered by core hydrogen fusion; fusion is
  explained, not simulated.

## 7. Known limitations

The model is deliberately limited. There is no hydrodynamics, radiative
transfer (beyond single-scattering shading), stellar-structure model, nuclear
network or N-body gravity. Early stages do not depend on mass, except the
central object's size and color. Post-main-sequence timing is not physical.
Giant radii, temperatures (3300–3600 K) and remnant temperatures (white dwarf
60,000 K, neutron star shown as a 40,000 K color) are illustrative choices.

## 8. References

- Salaris, M. & Cassisi, S. (2005), *Evolution of Stars and Stellar Populations*, Wiley.
- Duric, N. (2004), *Advanced Astrophysics*, Cambridge University Press.
- Demircan, O. & Kahraman, G. (1991), *Astrophysics and Space Science* 181, 313.
- Camposeo, F. (2025), "A new deepening of mass-radius empirical relation for main sequence stars", arXiv:2508.08845.
- Prša, A. et al. (2016), "Nominal values for selected solar and planetary quantities: IAU 2015 Resolution B3", *AJ* 152, 41.
- Wyman, C., Sloan, P.-P. & Shirley, P. (2013), "Simple Analytic Approximations to the CIE XYZ Color Matching Functions", *JCGT* 2(2).
- Sukhbold, T. et al. (2016), "Core-collapse supernovae from 9 to 120 solar masses based on neutrino-powered explosions", *ApJ* 821, 38.
- Shu, F. H. (1977), "Self-similar collapse of isothermal spheres and star formation", *ApJ* 214, 488 (the inside-out collapse concept).

Detail checked during development (2026-11-02): the luminosity relation and
its citations, the IAU 2015 B3 nominal T☉, the Demircan & Kahraman coefficients
(as reported by Camposeo 2025), the white-dwarf progenitor mass range, and the
non-monotonic NS/BH outcome. Bibliographic details for Sukhbold et al. (2016)
and Shu (1977) are from memory and should be confirmed before formal citation.

---

## 9. Proposed extension notes — not part of the current implemented model

The entries below are an expansion design register. They must not be merged into the claims in §§1–8 until the corresponding code, UI labels and verification evidence exist. `SCIENCE_NOTES.md` remains the record of what the executable actually calculates, prescribes and draws.

| Proposed capability | Scientific meaning that may be communicated | Current/required status language | What must not be claimed |
|---|---|---|---|
| Filamentary molecular-cloud morphology — **implemented 2026-11-08** (see §3) | Molecular clouds contain structured, non-uniform density regions; procedural filaments help visualise this morphology | `procedural density structure` unless calibrated/derived from data | A calculated turbulent molecular-cloud solution |
| Multiple dense cores | A cloud region can contain multiple condensations; a selected preset can demonstrate different core arrangements | `scenario preset` or `procedural core distribution` | Self-consistent gravitational fragmentation |
| Triggered-compression scene | An external compression front can be used to illustrate a possible route to enhanced density | `illustrative compression scenario` | Proof that the displayed shock caused star formation |
| Binary protostar scene | Two protostellar components and their shared environment can be visualised | `prescribed binary configuration` unless a validated orbit/fragmentation model is added | A resolved binary fragmentation or hydrodynamics calculation |
| Clustered formation | Several stars/cores can be shown at different stages | `procedural cluster scenario` | N-body cluster dynamics or a statistically representative cluster |
| Dipole magnetic-field lines — **implemented 2026-11-08** (see §3) | A specified analytical field geometry can be visualised with integrated field lines | `analytic field visualisation` | Self-consistent magnetohydrodynamics or a simulated magnetic dynamo |
| Stellar spots/prominences/flares — **implemented 2026-11-08** (see §3) | Surface activity can be explained with localised structures and time-dependent emission | `procedural/parametric surface activity` | Solved convection, reconnection or plasma transport |
| CME-like event | A surface-originating expanding structure can illustrate an ejection | `prescribed ejection visualisation` | A forecast or physical CME calculation |
| Rayleigh–Taylor-like ejecta fingers — **implemented 2026-11-08 as procedural morphology** (see §3) | Irregular finger-like morphology can illustrate instability-shaped interfaces | `instability-inspired procedural morphology` | A numerically solved Rayleigh–Taylor instability |
| Advanced volume scattering | More detailed light transport can improve the appearance of dusty gas | `rendering approximation` with documented parameters | A full wavelength-resolved radiative-transfer solution |
| Multi-wavelength observation mode — **implemented 2026-11-08 as an Observation view** (see §9.5) | Different bands reveal different observational information when genuine band data are used | `observational data view` with instrument/filter/source | A simulated X-ray/infrared/radio measurement when only a recoloured optical image is shown |
| Comparative evolution tracks — **partial (2026-11-08)**: simplified-model table only (VERIFICATION §8.12) | Different model/data tracks can compare mass-dependent evolution | Identify each track as `simplified model` or named reference dataset | A precise prediction from a simplified lifetime scaling |

### 9.1 Required provenance for observational data

For any observation shown in the application, record the source object, source institution, original page/identifier, instrument, filter/band or wavelength, observation date if available, creator/credit, usage terms, processing/composite notes, original dimensions, local derivative dimensions and checksum. A multi-filter composite must identify the channels and any assigned display colours. Do not infer a natural-colour appearance from a false-colour scientific composite.

### 9.2 Visual-field interpretation rules

A shader-generated density field may be exposed in scientific mode as a **normalized procedural density field**. It must not be labelled with a mass-density unit such as kg m⁻³ unless the code provides a physically defined calibration and the calibration is documented.

A vector field used to draw magnetic lines must be named and described (for example, analytic dipole field). A field-line tracer follows the specified mathematical field; it is not a material particle unless a separate particle equation is implemented.

A colour temperature selected for the appearance of a photosphere or remnant must be distinguished from a measured effective temperature. Keep the current provenance tags: `calculated`, `parameter`, `prescribed`, `model relation`, `reference`, `illustrative`, and `category`.

### 9.3 Reference datasets to investigate before integration

Potential sources for future reference tracks or observational material include MIST and PARSEC stellar-evolution resources, NASA/ESA/STScI image collections, and public astronomy archives such as MAST or HEASARC. These are candidates for investigation, not an assertion that data have been downloaded, validated, licensed, or integrated. Before use, verify the exact dataset version, variables, units, coverage, citation requirements, access conditions and redistribution terms.

### 9.4 Expansion science review gate

Before a new phenomenon is included in a scientific label or presentation, record: the physical concept; the simplified representation; any equation/field/parameter; the units; the assumptions; what is omitted; the exact phrase shown to the user; and the reference supporting the scientific explanation. If a phenomenon is only procedural, use wording such as `visual representation inspired by ...`, not `simulation of ...`.

### 9.5 Observation view as implemented (2026-11-08)

Seven authentic images, each with a JSON provenance record (source page, file URL,
credit, licence, instrument, band, band kind, image kind, processing notes, original
and local dimensions, derivative operations, intended use, retrieval date, SHA-256):
Pillars of Creation in visible light (Hubble) and in the near-infrared (Webb NIRCam), for
the cloud/collapse stages and as a genuine two-band pair of the same object; HH 211
(Webb), for protostar/jets; an SDO/AIA 304 Å limb prominence, for the main sequence;
the Southern Ring Nebula (Webb NIRCam + MIRI), for the white-dwarf ending; the Crab
Nebula (Hubble), for the neutron-star ending; and M87* (EHT, 1.3 mm), for the black-hole
ending. That last one is labelled as a supermassive, not stellar-mass, black hole.

The images are shown unmodified, each with its credit and band. False-colour and
composite products are identified as such. No band is simulated by recolouring.
The STELLARIS scene beside each image is captioned "a conceptual visualization, not
a 3D reconstruction". The images are references only: nothing in the model reads
them. Licences: CC BY 4.0 (ESA/Hubble, ESA/Webb, ESO), checked on each source page
and the institutions' usage pages on 2026-11-08, and public domain (NASA SVS). The
retrieved files are the 1024–1280 px "screensize" versions, sized for a side panel.

### 9.6 Real sky background (2026-11-08)

The background is the ESO/S. Brunier Milky Way panorama (eso0932a, CC BY 4.0), a real
visible-light photograph of the whole sky. We map it in galactic coordinates. The
scene's galactic plane follows the band that was already there, so the stellar nursery
sits against the Milky Way disc, where molecular clouds actually are. That placement is
a presentation choice, not a real sky position for the simulated star. A sparse layer
of procedural point stars is drawn over it so star cores stay sharp. The credit is on
screen at all times. `--procedural-sky` restores the fully procedural sky.
