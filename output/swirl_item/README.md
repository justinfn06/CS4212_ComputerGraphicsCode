# Adjustable swirl profile — version 6

Edit EDITABLE SETTINGS at the top of build_model.py, save, then run
`.\run_model.ps1` in PowerShell from this folder. Current saved settings produce approximately 68.891 × 147.697 × 10 mm. Import the STL in millimetres; bottom is at Z = 0.

## Diagrams

- swirl_item_v6_variable_map.png: part-by-part controls and thickness direction.
- swirl_item_v6_curve_controls.png: curve endpoints, handles and inner corners.

Diagrams regenerate from your geometry. S1/R1 labels start at 1; Python indexes
start at 0. Major shape changes may need label adjustments in diagram.py.

## Global controls: DEFAULTS

| Variable | Default | Meaning |
| --- | ---: | --- |
| shaft_length | 100 | Centreline distance from shaft top S0 to base B. |
| shaft_width | 2 | Width across the shaft in the bed plane. |
| shaft_thickness | None | Shaft front-to-back Z thickness. None follows body_thickness; set e.g. 12.0 for shaft only. |
| shaft_angle | -92.948 | Direction from S0 toward B. |
| foot_length | 28 | Block extension toward its free end. |
| foot_width | 10 | Constant width; grows only toward the head from the fixed base. |
| foot_angle | 180 | Direction from B toward F. |
| foot_thickness | None | Constant block Z thickness; None follows body_thickness. |
| foot_join_length | 3 | Length of thickness transition outside the block, into the shaft. |
| spring_width | 1.8 | Default width across the curved spring strip. |
| return_width | 4.5 | Default width of the angular return and end bar. |
| head_width | 70 | Horizontal scale of the head coordinate frame. |
| head_height | 80 | Vertical scale of the head coordinate frame. |
| head_rotation | 0 | Head rotation about its coordinate origin. |
| body_thickness | 10 | Front-to-back thickness, Z above the bed. |
| end_bar_length_scale | 0.8 | Multiplier on final endpoint distance. |
| end_bar_angle_offset | 0 | Rotation of final endpoint about previous point. |

Lengths are mm. Angles are counterclockwise degrees: 0 right, 90 up, 180 left,
-90 down. Shaft/foot angles are absolute; rotating the head does not rotate them.
The shaft stays attached to S0. Head scaling does not scale strip widths,
thicknesses or shaft length. Head width/height are coordinate-frame scales, not
exact exterior dimensions. Output is translated to X,Y >= 0.

## Spiral shape: HEAD_START and CURVE_SEGMENTS

Coordinates (u, v) go right and up. Before rotation, x = u * head_width,
y = v * head_height. Values outside 0–1 are allowed. Fractions in the defaults
preserve the trace; decimals also work. HEAD_START positions S0.

Each CURVE_SEGMENTS dictionary defines a cubic Bezier segment starting at the
preceding endpoint. Entry [0] ends at S1, [1] at S2, and so on:

| Field | Meaning |
| --- | --- |
| c1 | Handle controlling the direction leaving the start. |
| c2 | Handle controlling the direction approaching the end. |
| end | Endpoint position. |
| width_key | Global width to inherit, normally spring_width. |
| width_mm | Absolute endpoint width; None inherits width_key. |
| thickness_mm | Absolute endpoint Z; None inherits body_thickness. |
| steps | Sampled intervals, integer >= 2. More smooths the curve. |

Move endpoints for spacing/position, handles for curvature. Add/remove entries
for more/fewer bends or turns. For smooth tangents, align the preceding c2,
shared endpoint and next c1. Width/thickness transitions run between endpoints;
set both ends equal for a constant dimension along a segment.

```python
CURVE_SEGMENTS[1]['end'] = (0.25, 0.95)  # Move S2.
CURVE_SEGMENTS[3]['width_mm'] = 1.6      # Width at S4.
CURVE_SEGMENTS[2]['thickness_mm'] = 8.0  # Z at S3.
```

Edit dictionaries directly or put assignments below the settings block.

## Inner corners: RETURN_POINTS

Each entry has xy, width_mm and thickness_mm. R1 is [0], R5 is [4].
Move points to change lengths/angles/spacing. Add/remove entries for more/fewer
corners. The final endpoint is then modified by end_bar_length_scale and
end_bar_angle_offset; use 1 and 0 for direct coordinate control.

width_mm=None inherits return_width; thickness_mm=None inherits body_thickness.
Straight segments interpolate dimensions linearly. Add both c1 and c2 to curve
a segment, optionally with steps (default 32):

```python
RETURN_POINTS[4]['c1'] = (0.60, 0.34)
RETURN_POINTS[4]['c2'] = (0.35, 0.13)
RETURN_POINTS[4]['steps'] = 40
```

## Local overrides: LOCAL

| Variable | Point | Inherits when None |
| --- | --- | --- |
| shaft_base_width_mm | B | shaft_width |
| shaft_top_width_mm | S0 | shaft_width |
| foot_thickness_mm | Entire foot block | foot_thickness, otherwise body_thickness |
| shaft_base_thickness_mm | B | shaft_thickness (or body_thickness when None) |
| shaft_top_thickness_mm | S0 | shaft_thickness (or body_thickness when None) |

Different endpoint widths create shaft/curve taper. Different Z values create a
sloping top along those sections. The foot is always a constant-section block;
its thickness transition lies outside the block, controlled by foot_join_length.

## Generation: OPTIONS

- width_blend, thickness_blend: smooth or linear on sampled curves.
  Smooth eases dimension changes, not sharp outline corners.
- end_caps: butt stops at the end-bar centreline endpoint; square extends that tip
  by half its width. The foot remains a constant-width block with a flush rear edge.
- output_name: filename stem. Change it to retain experiments.
- write_preview, write_diagrams: True or False.

This generator controls a continuous ribbon attached to a constant-section foot block with a flat bottom. It does not
add holes, assemblies, textures or automatic edge fillets. Curved path
transitions can be constructed with curve handles. Crossed/touching outlines
are rejected before export.

## Running

```powershell
.\run_model.ps1
.\run_model.ps1 --spring-width 2.0 --body-thickness 12 --name trial_2mm_12mm
```

DEFAULTS entries accept command-line overrides with underscores changed to
hyphens. --depth aliases --body-thickness. Edit geometry lists, LOCAL and
OPTIONS directly in Python. Files with the same output name are replaced.

Outputs: STL, SCAD polyhedron, full settings JSON, preview and two diagrams.
JSON records settings for reference; it is not an automatic import preset.
Use Python for editing; SCAD contains the generated mesh.
STL generation uses the standard library. Pillow generates images and is
available in the bundled runtime. Keep diagram.py beside build_model.py.
On another computer use python build_model.py; the launcher has local paths.

## Validation and spring tuning

Checks cover outline intersections, closed edges, consistent winding, topology,
volume and serialized STL edges, including nonuniform thickness.
Test_model.py exercises defaults, thickness, shape edits and invalid controls.
Run it with the generator's Python interpreter.

These are geometry checks, not strength/fatigue tests. Material, printing and
deflection affect spring behavior. Compare physical prints one change at a time.
The reference white pad remains omitted.

## Adjust shaft thickness alone

Set `shaft_thickness` in DEFAULTS to a number such as `12.0`.
This changes front-to-back Z at both ends of the shaft. The foot stays at its own constant thickness; a short transition outside
the block and the first curve segment blend to the shaft thickness. Other spiral segments
keep their existing thickness. `shaft_width` independently controls XY width.
Set the two LOCAL shaft thickness overrides to None to use this single control.

```powershell
.\run_model.ps1 --shaft-thickness 12 --name shaft_12mm
```

## Foot with a flush back edge (version 6)

The foot's rear face is aligned with the OUTSIDE shaft edge. The rear base
anchor never moves when foot_length or foot_width changes. Length extends
only toward the free tip; width grows only toward the head. No foot material
extends behind the shaft's outside-edge line.

The foot retains parallel opposite sides and constant Z thickness. Its end
faces follow the shaft angle to preserve a smooth outside edge; at other than
90-degree foot/shaft angles, these ends are slightly slanted rather than square.
foot_width is measured perpendicular to the foot's long faces. The footprint
area remains foot_length * foot_width.

foot_angle remains an absolute direction in the bed plane. A foot parallel to
the shaft, one too short to span the shaft, or one too wide for its available
length is rejected. Increasing width consumes more of the shaft at the joint.
foot_join_length sets the thickness transition outside the block.

foot_thickness sets constant block Z; None follows body_thickness. The legacy
LOCAL['foot_thickness_mm'] is still an override for the whole foot.

```powershell
.\run_model.ps1 --foot-length 35 --foot-width 12 --foot-thickness 10 --name flush_foot
```
