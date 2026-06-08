// Arrow SDF fragment shader — shared source for RV and CR.
//
// Prepend compat_gl21.glsl (RV / OpenGL 2.1) or compat_webgl2.glsl
// (CR / WebGL 2) before compiling, then concatenate shape_common.glsl.
// Those files define FRAG_COLOR, IN_FRAG, and smoothColors() so this file
// contains no version-specific syntax.
//
// Renders an arrow (shaft + arrowhead) using a signed-distance field.
// The bounding quad is supplied by shape_quad.vert.  uStart and uEnd are
// the tail and tip of the arrow respectively, expressed in the same local
// coordinate space as vPosition.
//
// The arrow is modelled as the UNION of two convex shapes in arrow-local
// space, avoiding the sign-flip artifacts that occur near concave corners
// when using a single non-convex polygon SDF:
//
//   Shaft : axis-aligned box  [0..hb] x [-w1..+w1]
//   Head  : isosceles triangle with base at x = hb-w1 (overlaps shaft by
//           one shaft-width so min(sdBox,sdTriangle) is negative across the
//           entire junction — no visible seam between shaft and head).
//
// Union SDF = min(sd_shaft, sd_head) — exact for convex primitives.
//
// Uniforms:
//   uStart       — tail endpoint of the arrow (vec2, local coords)
//   uEnd         — tip  endpoint of the arrow (vec2, local coords)
//   uThickness   — half-width of the arrow shaft in local units
//   uInnerColor  — RGBA fill colour of the arrow body
//   uBorderColor — RGBA outline colour
//   uBorderWidth — outline half-width in local units
//
// SPDX-License-Identifier: Apache-2.0
// Copyright 2024 Autodesk, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

uniform vec2  uStart;
uniform vec2  uEnd;
uniform float uThickness;
uniform vec4  uInnerColor;
uniform vec4  uBorderColor;
uniform float uBorderWidth;
// See rectangle.frag for the uDpr contract.
uniform float uDpr;

IN_FRAG vec2 vPosition;

// Signed distance to an axis-aligned box centered at c with half-extents b.
float sdBox(vec2 p, vec2 c, vec2 b) {
    vec2 q = abs(p - c) - b;
    return length(max(q, 0.0)) + min(max(q.x, q.y), 0.0);
}

// Signed distance to a triangle defined by three vertices.
// Works for any winding order.
// Based on https://iquilezles.org/articles/distfunctions2d
// The MIT License — Copyright 2020 Inigo Quilez
float sdTriangle(vec2 p, vec2 p0, vec2 p1, vec2 p2) {
    vec2 e0 = p1 - p0, e1 = p2 - p1, e2 = p0 - p2;
    vec2 v0 = p  - p0, v1 = p  - p1, v2 = p  - p2;
    vec2 pq0 = v0 - e0 * clamp(dot(v0, e0) / dot(e0, e0), 0.0, 1.0);
    vec2 pq1 = v1 - e1 * clamp(dot(v1, e1) / dot(e1, e1), 0.0, 1.0);
    vec2 pq2 = v2 - e2 * clamp(dot(v2, e2) / dot(e2, e2), 0.0, 1.0);
    float s = sign(e0.x * e2.y - e0.y * e2.x);
    vec2 d = min(min(
        vec2(dot(pq0, pq0), s * (v0.x * e0.y - v0.y * e0.x)),
        vec2(dot(pq1, pq1), s * (v1.x * e1.y - v1.y * e1.x))),
        vec2(dot(pq2, pq2), s * (v2.x * e2.y - v2.y * e2.x)));
    return -sqrt(d.x) * sign(d.y);
}

// Signed distance to an arrow from a (tail) to b (tip).
// w1 = shaft half-width, w2 = head base half-width (typically 2.5 * w1).
//
// Rendered as the union (min) of:
//   - A shaft box : [0..hb]        x [-w1..+w1]
//   - A head triangle base at hb-w1 (one shaft-width before hb) so the
//     triangle SDF is already negative at x=hb, eliminating the seam.
float sdArrow(vec2 p, vec2 a, vec2 b, float w1, float w2) {
    vec2  ba = b - a;
    float l  = length(ba);

    if (l < 1.0e-5)
        return length(p - a) - w1;

    // Transform p into arrow-local space: x along ba, y perpendicular.
    p -= a;
    p  = mat2(ba.x, -ba.y, ba.y, ba.x) * p / l;

    // Shaft ends at hb; the head base is moved one shaft-width to the left
    // so the two primitives overlap and min() is negative across the junction.
    float hb      = max(l - 2.0 * w2, 0.0);
    float hb_head = max(hb - w1,       0.0);

    // Shaft: box [0..hb] x [-w1..+w1]
    float d_shaft = sdBox(p, vec2(hb * 0.5, 0.0), vec2(hb * 0.5, w1));

    // Head: triangle  (hb_head,-w2) -> (l,0) -> (hb_head,+w2)
    float d_head = sdTriangle(p,
                               vec2(hb_head, -w2),
                               vec2(l,        0.0),
                               vec2(hb_head,  w2));

    return min(d_shaft, d_head);
}

void main()
{
    float headHalfWidth = uThickness * 2.5;
    vec2  p  = vPosition;
    float sd = sdArrow(p, uStart, uEnd, uThickness, headHalfWidth);

    float aa = fwidth(vPosition.x) * uDpr;
    vec4 fadeOut = vec4(uBorderColor.rgb, 0.0);
    vec4 effectiveInner = (uInnerColor.a > 0.0) ? uInnerColor : fadeOut;
    if (sd > aa) {
        discard;
    } else if (sd > -uBorderWidth) {
        FRAG_COLOR = smoothColors(uBorderColor, fadeOut, sd, aa);
    } else {
        FRAG_COLOR = smoothColors(effectiveInner, uBorderColor, sd + uBorderWidth, aa);
    }
}
