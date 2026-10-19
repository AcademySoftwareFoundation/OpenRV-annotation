# Shaders

Shared GLSL source for the annotation renderer, compiled for both RV
(OpenGL 2.1 / GLSL 1.20) and CR (WebGL2 / GLSL ES 3.00). Each shader file is
version-agnostic: it's compiled by prepending one of the two compatibility
headers below, which `#define` the handful of macros (`IN_VERT`, `OUT_VERT`,
`IN_FRAG`, `FRAG_COLOR`, `SAMPLE2D`, ...) that differ between the two GLSL
dialects.

## Compatibility headers

| File | Target | Notes |
| --- | --- | --- |
| `compat_gl21.glsl` | RV (OpenGL 2.1, GLSL 1.20) | Maps macros to `attribute`/`varying`/`gl_FragColor`/`texture2D`. |
| `compat_webgl2.glsl` | CR (WebGL2, GLSL ES 3.00) | Maps macros to `in`/`out`/a declared `fragColor` output/`texture`. |

Prepend exactly one of these to a shader source before compiling; the shader
files themselves contain no `#version` directive or version-specific syntax.

## Shapes

SDF-based (signed-distance-field) vector annotation primitives — arrows,
ellipses, lines, rectangles. All share one vertex shader and a small helper
file for anti-aliased edge blending.

| File | Role |
| --- | --- |
| `shape_quad.vert` | Shared vertex shader for all shapes; transforms a bounding-box quad into clip space and passes local-space position to the fragment shader for SDF evaluation. |
| `shape_common.glsl` | Shared helper (`smoothColors`) for anti-aliased inner→border→outer colour blending across an SDF boundary; concatenated into each shape fragment shader. |
| `arrow.frag` | Draws an arrow (shaft + head) as the union of a box and triangle SDF. |
| `ellipse.frag` | Draws an ellipse (or circle, via a fast path) from an analytical SDF. |
| `line.frag` | Draws a straight line segment as a rounded-capsule SDF (border colour only, no fill). |
| `rectangle.frag` | Draws a rectangle, optionally with independently-rounded corners, from an SDF. |

## Stamps

Brush "dab" primitives used to build up a paint stroke by repeatedly
stamping a small textured quad along the stroke path. Each variant has its
own vertex/fragment pair.

| File | Role |
| --- | --- |
| `stamp_hard.vert` / `stamp_hard.frag` | Hard-edged brush: the stamp shape comes entirely from the `brushTip` texture's red channel, with `fwidth`/`smoothstep` edge AA. |
| `stamp_soft_procedural.vert` / `stamp_soft_procedural.frag` | Soft brush with no texture: alpha is a fully procedural Gaussian falloff from the stamp centre, always maximally soft. |
| `stamp_texture.vert` / `stamp_texture.frag` | Textured brush with tunable hardness: multiplies a pattern texture by a procedural circular falloff with a solid core (sized by `hardness`) and a Gaussian fade to the edge; also carries per-stamp `Opacity` for support of opacity jitter. |
