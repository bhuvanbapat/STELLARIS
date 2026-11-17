# STELLARIS — External Asset Sources

The register of every external asset: source, creator, licence, attribution and
intended use. Machine-readable provenance (checksums, dimensions, processing) is in the
JSON sidecar beside each file. `tools/check_observations.py` validates those sidecars
and runs as a CTest test. Code dependencies are listed in `cmake/Dependencies.cmake`.

## 1. Assets used in the application

| Asset | Local path | Creator / credit | Licence / terms | Use |
|---|---|---|---|---|
| Milky Way panorama, 6000×3000 (eso0932a) | `assets/sky/` | ESO/S. Brunier | CC BY 4.0, credit shown on screen | Background sky of every stage |
| Cassiopeia A 3D reconstruction (ASCII VTK) | `assets/data/casa/` | NASA/CXC/SAO; T. Delaney et al. 2010, ApJ 725, 2038 | No copyright asserted by CXC; NASA media guidelines; acknowledgement given | Baked at build time into a 192³ emission volume: ejecta of the neutron-star remnant |
| SDO/HMI synoptic magnetogram, Carrington rotation 2284 (2024 May–Jun) | `assets/data/sun/` | Courtesy of NASA/SDO and the HMI science team (JSOC) | Public domain (NASA data) | Real sunspots, plage, flare site, prominence anchors and coronal brightening of Sun-like stars |
| Astronomy reference library: 75 files (ESA/Webb, ESA/Hubble, ESO, NASA/SDO) | `assets/astronomy/` (not committed; `tools/acquire_assets.py`) | per file, see `assets/astronomy/ASSET_CATALOG.md` | CC BY 4.0 (ESA/ESO) or public domain (NASA) | Reference library; 63 display copies in the Observation view |
| Pillars of Creation, visible (heic1501a) | `assets/observations/` | NASA, ESA/Hubble and the Hubble Heritage Team | CC BY 4.0 | Observation view, stages 1–2 |
| Pillars of Creation, NIRCam (weic2216a) | `assets/observations/` | NASA, ESA, CSA, STScI; J. DePasquale, A. Koekemoer, A. Pagan | CC BY 4.0 | Observation view, stages 1–2 |
| HH 211 (weic2322a) | `assets/observations/` | ESA/Webb, NASA, CSA, T. Ray (DIAS) | CC BY 4.0 | Observation view, stages 3–4; reference for the jet model |
| Solar limb prominence, AIA 304 Å (SVS 4277) | `assets/observations/` | NASA SVS, SDO Science Team, Virtual Solar Observatory | Public domain | Observation view, stage 5 |
| Southern Ring Nebula (weic2207a) | `assets/observations/` | NASA, ESA, CSA, STScI, Webb ERO Production Team | CC BY 4.0 | Observation view, white-dwarf ending; reference for nebula stratification |
| Crab Nebula (heic0515a) | `assets/observations/` | NASA, ESA, A. Loll / J. Hester (ASU); ack. D. De Martin | CC BY 4.0 | Observation view, neutron-star ending |
| M87* (eso1907a) | `assets/observations/` | EHT Collaboration | CC BY 4.0 | Observation view, black-hole ending (labelled supermassive) |

## 2. Evaluated, not used (and why)

| Candidate | Finding | Decision |
|---|---|---|
| Cas A 3D model, Smithsonian 3D (3d.si.edu) | The page returned HTTP 403 to automated access. | Used the same reconstruction from the Chandra X-ray Center instead. |
| Cas A OBJ/STL print models (Chandra, NASA 3D Resources) | 600k-triangle meshes merged for printing, without element separation. | Used the per-element VTK surfaces: same data, keeps Si/Fe/IR/optical separate. |
| Larger versions of the observation images | The panel displays at ~700 px; the stored files are 1024–1280 px. | Not needed: more memory, no visible gain. |
| Full-disk solar images (e.g. SDO/HMI continuum) as a star texture | One projected hemisphere of one star; it cannot cover a rotating 3D star of arbitrary mass and age without inventing the far side. | Not used as a texture. The photosphere is procedural, after these observations (granulation, limb darkening, spots, faculae). |
| Herbig–Haro jet and planetary-nebula images as textures | 2D projections, many of them false colour; using them as volume density would misrepresent the data (CLAUDE.md §19.7). | Used as references for the procedural media; shown unmodified in the Observation view. |

## 3. Scientific references for the procedural media (not distributed assets)

- **Jets:**
  - Reipurth & Bally 2001, ARA&A 39, 403 (Herbig–Haro flows).
  - Raga et al. 1990, ApJ 364, 601 (internal working surfaces).
  - Hartigan et al. 2011, ApJ 736, 29 (HH jet proper-motion movies).
- **Disc:**
  - Chiang & Goldreich 1997, ApJ 490, 368 (flared disc surface layers).
  - ALMA Partnership 2015, ApJ 808, L3 (HL Tau rings).
- **Infall:** Pineda et al. 2020, Nature Astronomy 4, 1158 (accretion streamer).
- **Photosphere:**
  - Neckel & Labs 1994, Solar Physics 153, 91 (solar limb darkening).
  - Chiavassa et al. 2011, A&A 535, A22 (red supergiant convection).
- **Sonification:**
  - Kjeldsen & Bedding 1995, A&A 293, 87.
  - Huber et al. 2011, ApJ 743, 143 (asteroseismic scaling).

## 4. Rules

Every new external asset gets a JSON sidecar and a row in §1 before it ships. Licences
are taken from the asset's own page and the institution's usage policy. An asset whose
licence cannot be confirmed is not distributed; at most it is listed in §2 as a
reference.
