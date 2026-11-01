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
