# STELLARIS — Scientific Model and Assumptions

## 1. Purpose

This document defines the scientific principles, modeling boundaries, assumptions, and terminology used in STELLARIS.

STELLARIS presents the formation and evolution of stars through an interactive 3D visualization.

The application must communicate scientifically defensible concepts without pretending to reproduce the complete physics of stellar formation or stellar interiors.

The scientific model must remain consistent with the application's visual narrative.

**Scientific credibility takes priority over visually impressive but physically misleading representations.**

This document is authoritative for scientific interpretation. Implementation details must follow its requirements unless the user explicitly approves a revision.

---

## 2. Modeling Philosophy

STELLARIS is a scientific visualization with a limited computational model.

It is not a research-grade astrophysics simulator.

The application may use simplified equations, parameterized processes, procedural animation, and artistic effects to communicate stellar phenomena.

However, it must distinguish clearly between:

1. Physical principles represented by the model.
2. Quantities calculated by implemented equations.
3. Parameters selected by the user or configured in the application.
4. Procedural approximations used to create visual motion.
5. Artistic effects that communicate a concept without simulating its underlying physics.

Do not describe an artistic effect as a calculated physical result.

Do not claim that the application predicts the behavior of a real star unless the relevant model has been implemented and validated appropriately.

---

## 3. Stellar Formation: Scientific Overview

The principal formation narrative is:

1. Molecular cloud.
2. Gravitational collapse.
3. Protostar formation.
4. Accretion disc and bipolar jets.
5. Main-sequence star.
6. Mass-dependent stellar evolution and final remnant.

These stages provide the application's educational structure.

They must not be presented as six universally distinct, sharply separated physical events. Real stellar formation and evolution involve overlapping processes and timescales that are compressed for visualization.

The timeline is an explanatory representation of the process, not a literal simulation clock.

Unless explicitly implemented, the application must not imply that the elapsed seconds of playback correspond directly to physical time in years.

---

## 4. Stage 1 — Molecular Cloud

### Scientific concept

Stars form within cold, dense regions of molecular clouds composed primarily of gas, with dust also present.

Clouds contain variations in density, temperature, composition, and motion.

A sufficiently dense region may become gravitationally unstable and collapse, subject to its physical conditions.

### Representation in STELLARIS

The visualization may represent the cloud using:

* Procedural density fields.
* Volumetric rendering.
* Particles.
* Procedural geometry.
* Layered structures.
* Other suitable graphics techniques.

The visual density field does not need to represent a fully calculated hydrodynamic solution.

If the application uses an artistic density distribution, document it as a procedural approximation.

### Scientific constraints

Do not imply that:

* All molecular clouds immediately form stars.
* Every region of a cloud collapses at the same rate.
* The displayed cloud structure is a measured astronomical object.
* Particle positions necessarily correspond to real gas molecules.

If the cloud is fictional or procedurally generated, describe it as an illustrative molecular cloud.

---

## 5. Stage 2 — Gravitational Collapse

### Scientific concept

Gravity can cause sufficiently dense regions of a molecular cloud to contract.

As a collapsing region becomes denser, its gravitational potential energy is converted through the dynamics of collapse into other forms, including thermal energy.

The detailed evolution depends on factors such as pressure, rotation, magnetic fields, turbulence, and the cloud's initial conditions.

### Representation in STELLARIS

A simplified model may represent collapse through:

* A prescribed contraction parameter.
* A changing density distribution.
* A controlled inward velocity field.
* An evolving central concentration.
* A parameterized increase in central emission.

These effects may be driven by a normalized timeline variable rather than by a full numerical fluid simulation.

### Scientific constraints

Do not claim to solve the complete equations of gravitational collapse unless those equations are actually implemented.

Do not assume that every point of the cloud moves radially inward in the same way.

Do not equate increased visual brightness with a directly calculated temperature unless a defined model supports that relationship.

Clearly identify prescribed animation as a visualization approximation.

---

## 6. Stage 3 — Protostar Formation

### Scientific concept

A protostar forms as material accumulates in the central region of a collapsing cloud.

Accretion and contraction contribute to the protostar's evolution.

The protostar's internal conditions change as its mass and structure evolve.

A protostar is not simply a fully developed main-sequence star with a different visual appearance.

### Representation in STELLARIS

The application may represent protostar formation using:

* A central object whose size and appearance evolve.
* Material moving toward the central region.
* A changing surrounding density distribution.
* Parameterized emission.
* A controlled transition from the collapsing cloud.

### Scientific constraints

Do not claim to calculate a real protostar's internal temperature, pressure, luminosity, or mass unless the relevant equations and assumptions have been implemented.

Do not imply that a visually bright object has necessarily reached the main sequence.

The transition from collapse to protostar formation is simplified for the application's presentation.

---

## 7. Stage 4 — Accretion Disc and Bipolar Jets

### Scientific concept

Angular momentum can cause infalling material to form a rotating accretion disc around a young stellar object.

Accretion discs redistribute angular momentum and can transport material inward.

Some young stellar objects produce bipolar outflows or jets associated with their accretion environment and magnetic processes.

These phenomena involve complex physics that is beyond the scope of a basic Computer Graphics visualization.

### Representation in STELLARIS

The visualization may use:

* Procedural disc geometry.
* Angular animation.
* Parameterized radial density.
* Shader-based emission.
* Particle streams.
* Procedural jet structures.
* Simplified velocity fields.

### Scientific constraints

Do not claim to calculate magnetohydrodynamic jet launching or realistic accretion dynamics unless these processes are explicitly modeled.

Do not imply that all infalling material is expelled by the jets.

Do not represent the disc as a stationary decorative ring while claiming to simulate accretion dynamics.

If the disc and jets are animated procedurally, identify them as simplified visual representations.

---

## 8. Stage 5 — Main-Sequence Star

### Scientific concept

A main-sequence star is in a stage of sustained hydrogen fusion in its core.

Its properties depend on mass, composition, and evolutionary state.

Main-sequence stars differ substantially in luminosity, radius, temperature, and lifetime.

A star's visual color may provide qualitative information about its surface temperature, but visual rendering is not itself a physical temperature calculation.

### Representation in STELLARIS

The application may use:

* A parameterized luminous surface.
* Procedural surface detail.
* Emission and lighting.
* An illustrative stellar color.
* Simplified visual atmospheric effects.

If the application selects a representative star, its assumed parameters should be documented.

### Scientific constraints

Do not claim to simulate nuclear fusion, stellar-interior energy transport, or detailed stellar structure unless those calculations have actually been implemented.

Do not imply that a surface shader reproduces real stellar convection.

Do not present a stylized surface color as a measured temperature.

The application may explain hydrogen fusion scientifically without simulating the reaction network.

---

## 9. Stage 6 — Mass-Dependent Stellar Evolution

### Scientific concept

Initial stellar mass is a major determinant of stellar evolution.

Stars of different masses follow different evolutionary pathways.

The final outcome also depends on factors such as composition, mass loss, rotation, and binary interaction.

The application's primary model may simplify these factors, but it must not imply that initial mass is the only influence in every real stellar system.

### Required illustrative pathways

The visualization must communicate at least the following broad outcomes.

#### A. Low- and intermediate-mass pathway

A representative pathway may include:

* Evolution away from the main sequence.
* Expansion into a giant phase.
* Loss of outer material.
* Formation of a white dwarf.

The precise evolutionary sequence depends on the selected representative stellar model.

Do not imply that every star follows identical timing or structural evolution.

#### B. High-mass pathway

A representative pathway may include:

* Evolution away from the main sequence.
* Advanced burning stages.
* Core collapse for stars that reach the relevant conditions.
* A supernova event where appropriate.
* Formation of a neutron star or black hole, depending on the progenitor and subsequent physical processes.

Do not imply that every high-mass star necessarily produces an observable supernova of the same type.

Do not imply that every massive star inevitably becomes a black hole.

### Scientific constraints

The exact mass ranges, representative stellar parameters, and pathway-selection rules must be specified before implementation.

Do not invent universal mass thresholds.

Do not use a single unsupported threshold to suggest that real stellar evolution has a perfectly sharp division between all possible outcomes.

If the model uses simplified mass categories, label them as illustrative categories.

---

## 10. Mass and Evolution Parameters

The initial stellar mass may be represented as an application parameter if the approved project specification requires user interaction with stellar evolution.

If a mass control is implemented, it must have a defined meaning and units.

Where appropriate, stellar mass should be expressed in solar masses:

$$
M_\odot \approx 1.9885 \times 10^{30}\ \text{kg}
$$

This value is a reference constant, not a requirement to calculate a complete stellar model.

The application must define:

* The permitted mass range.
* The meaning of the selected mass.
* How the mass affects the simplified evolutionary pathway.
* Which displayed values are calculated and which are illustrative.
* The assumptions governing pathway selection.

Do not add a mass slider until the model can explain what the selected mass actually changes.

---

## 11. Numerical Quantities and Units

Every numerical quantity shown to the viewer must have a defined interpretation.

Examples of quantities that may be appropriate include:

* Initial stellar mass.
* Illustrative radius.
* Normalized density.
* Normalized collapse progress.
* Timeline position.
* Playback speed.

These examples are not a requirement to display every quantity.

### Rules

1. Use SI units where appropriate for physical quantities.
2. Use solar units where they make stellar comparisons clearer.
3. Label dimensionless normalized values as dimensionless.
4. Do not assign physical units to arbitrary rendering parameters.
5. Do not imply that a normalized animation variable represents physical time.
6. Do not fabricate numerical precision.
7. Do not display calculated values unless the calculation is implemented and its assumptions are documented.
8. Do not invent measurements from real astronomical objects.

If a value is an illustrative parameter rather than a prediction, communicate that distinction.

---

## 12. Simplified Mathematical Models

Simplified mathematical models are permitted when they improve scientific coherence without introducing unnecessary complexity.

Possible models include:

* Parameterized contraction.
* Prescribed radial velocity fields.
* Normalized density evolution.
* Interpolation between defined visual states.
* Simplified mass-dependent pathway selection.
* Parameterized luminosity or radius for an illustrative star.

Before implementing any equation, document:

1. What physical concept it represents.
2. Its assumptions.
3. Its inputs and outputs.
4. Its units.
5. Its limitations.
6. How its output affects the visualization.

Do not introduce equations solely to make the application appear scientific.

Do not implement a formula from memory when its applicability or assumptions are uncertain.

Verify important equations and constants using authoritative scientific references.

---

## 13. Deterministic Visualization

The timeline must reproduce consistent visual states.

Procedural animation may be driven by:

* Normalized timeline position.
* Explicit model parameters.
* Deterministic procedural functions.
* Defined transition functions.
* A documented simplified model.

The same configuration and timeline position should produce the same intended visual state.

Avoid relying exclusively on accumulated frame-by-frame updates when the viewer must be able to seek backward and forward.

If a numerical simulation is introduced, distinguish its internal state from the artistic presentation layer.

---

## 14. Scientific References and Verification

Scientific claims and important numerical constants must be supported by credible references.

Prefer:

* NASA.
* ESA.
* Peer-reviewed astronomical literature.
* University astrophysics resources.
* Authoritative textbooks and established scientific reference works.

For important claims, record:

* The claim being supported.
* The source.
* The relevant equation or explanation, where applicable.
* The assumptions and limitations.
* The way the claim is represented in STELLARIS.

Do not invent citations, paper titles, authors, measurements, or source details.

If a scientific claim cannot be verified, either omit it, qualify it, or identify it as unresolved.

Distinguish verified scientific information from implementation choices and artistic interpretation.

---

## 15. Validation Boundaries

Validation must match the complexity of the implemented model.

For a procedural visualization, appropriate checks may include:

* Consistent parameter ranges.
* Deterministic output.
* Correct units and labels.
* Valid transitions between defined stages.
* Correct mass-category mapping under the documented rules.
* No contradictory explanatory text.

For an actual numerical model, additional checks may be required, depending on the equations implemented.

Do not claim scientific validation merely because:

* The application compiles.
* The animation looks plausible.
* The output resembles an astronomical photograph.
* The displayed numbers seem reasonable.

A visually convincing effect is not proof of physical accuracy.

---

## 16. Scientific Presentation to the Viewer

Scientific information must remain concise and understandable.

For each stage, communicate:

* The principal physical process.
* Its role in stellar formation or evolution.
* The key distinction from the preceding stage.
* Any important modeling limitation.

Do not overload the interface with textbook-length explanations.

Detailed assumptions should remain in the project documentation, with essential qualifications included in the application where necessary.

---

## 17. Explicitly Prohibited Scientific Misrepresentation

The application must not:

* Claim to implement full astrophysical simulation when it does not.
* Present arbitrary particle movement as a validated fluid solution.
* Present procedural brightness as calculated luminosity without a model.
* Present shader colors as measured temperatures.
* Claim to solve complete gravitational collapse when using prescribed animation.
* Claim to simulate nuclear fusion when only a visual surface effect exists.
* Treat every star as following the same evolutionary pathway.
* Treat the final evolutionary outcome as determined by one universal mass threshold.
* Present arbitrary animation time as real stellar time.
* Fabricate numerical results or scientific validation.
* Invent references to support unsupported claims.

---

## 18. Scientific Acceptance Criteria

The scientific model is acceptable when:

* [ ] The six-stage narrative is scientifically coherent.
* [ ] The molecular-cloud stage is not presented as a guaranteed star-forming event for every cloud.
* [ ] Gravitational collapse is described with appropriate qualifications.
* [ ] Protostar formation is distinguished from the main-sequence stage.
* [ ] The accretion disc and jets are represented without unsupported claims of detailed physical simulation.
* [ ] The main-sequence stage is explained through sustained core hydrogen fusion.
* [ ] Different evolutionary pathways are represented.
* [ ] White dwarfs, neutron stars, and black holes are not treated as interchangeable outcomes.
* [ ] Mass-dependent behavior follows documented model assumptions.
* [ ] Numerical quantities have defined meanings and appropriate units.
* [ ] Artistic effects are distinguished from calculated physical quantities.
* [ ] Important scientific claims have credible references.
* [ ] Known limitations are documented.
* [ ] No unsupported scientific accuracy claims are made.

Do not mark a criterion complete without evidence.

---

## 19. Final Principle

STELLARIS should simplify astrophysical processes without misrepresenting them.

The objective is not to simulate everything.

The objective is to implement a limited model whose behavior is understandable, whose assumptions are explicit, and whose visual presentation communicates the science effectively.

**A scientifically honest approximation is better than a visually convincing claim that the application cannot substantiate.**

---

## 20. Extension models and scientific boundaries

This section adds possible scientific-visualisation extensions to the existing model. It does not silently upgrade the current prescribed animation into a physical simulation. Any extension that becomes active must be reflected in `SCIENCE_NOTES.md` with its actual implementation status and in `VERIFICATION.md` with matching evidence.

### 20.1 Multi-scale molecular-cloud representation

A more structured cloud may contain a large-scale envelope, filaments, dense cores, globules, cavities and dust lanes. In the first implementation, these may be constructed as deterministic procedural components of a normalized density field. Their purpose is to show the spatial hierarchy of a cloud, not to claim a solved turbulent molecular-cloud initial condition.

If a normalized visual density field is denoted by \(\rho_n(\mathbf{x})\), the subscript is deliberate: it is a dimensionless rendering/model quantity unless a physical calibration is explicitly defined. Do not write \(\rho\) in kg m\(^{-3}\) merely because the display resembles a density map.

The procedural field should have a stable seed and a documented mapping from configuration parameters to output. Separate broad envelope, filament, core and small-scale components in the implementation so each can be inspected and validated. Avoid claiming that the procedural combination solves the equations of turbulent gas dynamics or self-gravity.

### 20.2 Temperature and velocity display fields

A scientific inspection view may show temperature or velocity only if the field has a defined source:

- **Reference temperature:** a representative value taken from an identified scientific source and displayed as context, not assigned to every voxel.
- **Prescribed temperature field:** a deterministic field used for visual explanation, explicitly labelled as prescribed/illustrative.
- **Calculated temperature field:** only allowed when an implemented model computes it with documented inputs, equations, units, numerical method and validation.
- **Velocity field:** must come from the scene's defined trajectory/model equations. If particle trajectories are merely art-directed, the overlay must say `prescribed velocity/direction` or avoid physical units.

Never derive a physically labelled temperature or velocity solely from the colour or screen-space movement of a shader effect.

### 20.3 Analytic magnetic-field visualisation

A dipole field is an acceptable first visualisation model for a selected stellar magnetic geometry. In an ideal vacuum dipole representation, one common form is

\[
\mathbf{B}(\mathbf{r}) = \frac{\mu_0}{4\pi r^3}\left[3(\mathbf{m}\cdot\hat{\mathbf r})\hat{\mathbf r}-\mathbf m\right],
\]

where \(\mathbf m\) is the dipole moment, \(r=|\mathbf r|\), and \(\hat{\mathbf r}\) is the radial unit vector. The formula is a field geometry model; it does not describe a realistic stellar magnetosphere in all regions, nor does it include plasma currents, rotation, reconnection or MHD feedback.

A field line may be generated by integrating a curve whose tangent follows the local field direction, for example

\[
\frac{d\mathbf{x}}{ds}=\pm\frac{\mathbf B(\mathbf x)}{|\mathbf B(\mathbf x)|},
\]

with a documented numerical step, seed location, stopping radius, maximum path length and singularity handling. The resulting line is a mathematical integral curve. It is not a material trajectory. If the field is normalized for visual display, say so; if a field strength is displayed in tesla, its dipole moment and units must be defined.

Do not claim that such a dipole line system launches the existing jets or predicts stellar activity. It is an explanatory overlay unless the field is explicitly coupled to a validated physical model.

### 20.4 Stellar surface activity representations

Photosphere patterns, spots, prominences, flares and coronal structures may be used to explain observed forms of stellar activity. A real-time shader may parameterize their appearance and evolution, but must not claim to solve:

- convection or magnetoconvection;
- magnetic flux emergence;
- magnetic reconnection;
- plasma heating and transport;
- radiative transfer through a realistic stellar atmosphere;
- coronal mass ejection initiation or propagation.

A flare/ejection event should have deterministic position and timing. Its event controls are rendering/presentation parameters unless a physical energy model is actually implemented. Do not label a shader's brightness as a measured flare energy.

### 20.5 Instability-inspired supernova morphology

A procedural supernova remnant can use a perturbed shell, stratified layers, clumps, and elongated fingers to communicate that real ejecta can develop complex structure. Such a visual may be described as `Rayleigh–Taylor-like` or `instability-inspired` only when the resemblance and limitation are explained.

A genuine Rayleigh–Taylor calculation requires a defined fluid model, density contrast, acceleration, interface conditions, governing equations, numerical method, resolution study and validation. Seeded noise displaced along a shell is not such a calculation. The visual system should instead document its procedural field, seed, perturbation scales and time evolution.

### 20.6 Formation-scenario semantics

Optional formation configurations should be described carefully:

- **Binary configuration:** two condensations and their subsequent prescribed/analytical orbital motion. This does not itself simulate fragmentation or binary capture.
- **Cluster configuration:** several cores/stars at different positions or stages. Unless N-body dynamics are implemented, their mutual gravitational interactions are not being solved.
- **Triggered-compression configuration:** a visual compression front and subsequent denser region. It demonstrates a possible conceptual sequence, not causality for a real region.
- **Massive-star configuration:** uses the existing mass-dependent model and visual feedback additions. The relation between mass, final outcome and supernova visibility remains uncertain in real stellar populations.

Scenario configuration must state which properties are physical inputs, illustrative controls, or fixed presentation choices. Do not interpret the presence of a binary or cluster scene as a more complete physical model by itself.

### 20.7 Multi-wavelength observations

A multi-wavelength mode should display observational products from their actual wavelength band/instrument or an explicitly documented synthetic representation. It must distinguish:

- raw/calibrated or processed observation;
- multi-filter composite with assigned colour channels;
- synthetic rendering from a model;
- simple colour-map applied for visualization.

A false-colour infrared or X-ray composite must not be called a natural-colour image. Applying a hue filter to an optical photograph does not create an infrared, ultraviolet, X-ray or radio observation. Preserve instrument/filter labels and source credits.

If the application renders a synthetic wavelength view, specify the physical emission/absorption variable represented, the transfer/colour-mapping approximation, units or normalization, and all omitted processes. If these cannot be defined, present it as an artistic alternate colour view rather than a wavelength mode.

### 20.8 Comparative evolutionary tracks

The current mass-luminosity/radius/lifetime relations are approximate scaling relations. A future comparison mode may present these relations for multiple masses or load published evolutionary tracks. These must remain separate data sources:

1. **Current analytical approximation:** identify the existing equations and ranges of applicability.
2. **Published track:** identify the exact dataset, version, metallicity, initial mass, variables, units, interpolation method and citation.
3. **Illustrative pathway label:** show the simplified category and uncertainty boundaries without presenting them as a deterministic universal outcome.

Do not splice a published dataset into the existing model without documenting how the data are selected/interpolated and what the transition means. Track data outside their domain must not be extrapolated silently.

### 20.9 Scientific provenance and image use

Every observation must preserve source identifier, original title, source institution, creator/credit, use terms, observation/instrument, band/filter, wavelength when supplied, processing notes, image dimensions and the local transformation history. Asset-level rights must be checked individually. An image that can be viewed in a public archive is not automatically permitted to be redistributed inside a public repository.

Use observational images as a reference panel, a labelled image plane, or an explicitly described comparison. Do not infer the actual 3D distribution of gas from a 2D image without a reconstruction method and supporting data. Do not use a nebula photograph as if it were a physically calibrated PBR albedo map.

### 20.10 Scientific acceptance criteria for extensions

An extension is scientifically acceptable only when:

- [ ] every field has a defined meaning, unit or normalized status;
- [ ] the implementation is distinguished from the physical concept it represents;
- [ ] any analytical formula has documented assumptions and domain;
- [ ] procedural morphology is not labelled as a numerical physical solution;
- [ ] magnetic field lines are not confused with particle paths;
- [ ] a multi-wavelength view uses the correct source band or is explicitly labelled synthetic/illustrative;
- [ ] external data have complete provenance and usage review;
- [ ] evolutionary tracks identify their source/model and interpolation range;
- [ ] uncertain stellar fate boundaries remain qualified;
- [ ] all claims shown in the UI match what the executable actually computes.
