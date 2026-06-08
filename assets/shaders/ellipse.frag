// Ellipse SDF fragment shader — shared source for RV and CR.
//
// Prepend compat_gl21.glsl (RV / OpenGL 2.1) or compat_webgl2.glsl
// (CR / WebGL 2) before compiling, then concatenate shape_common.glsl.
// Those files define FRAG_COLOR, IN_FRAG, and smoothColors() so this file
// contains no version-specific syntax.
//
// Renders an ellipse (or circle, via a fast path) using a signed-distance
// field.  The bounding quad is supplied by shape_quad.vert.
//
// Uniforms:
//   uCenter      — centre of the ellipse in local (shape) coordinates
//   uWidth       — full width  (x-axis diameter)
//   uHeight      — full height (y-axis diameter)
//   uInnerColor  — RGBA fill colour (alpha = 0 for hollow shapes)
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

uniform vec2  uCenter;
uniform float uWidth;
uniform float uHeight;
uniform vec4  uInnerColor;
uniform vec4  uBorderColor;
uniform float uBorderWidth;
// See rectangle.frag for the uDpr contract.
uniform float uDpr;

IN_FRAG vec2 vPosition;

// Circle Signed Distance, taken from: https://www.shadertoy.com/view/3ltSW2
//
// The MIT License
// Copyright © 2020 Inigo Quilez
// Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions: The above copyright
// notice and this permission notice shall be included in all copies or
// substantial portions of the Software. THE SOFTWARE IS PROVIDED "AS IS",
// WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
// TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
// CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
// SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
// Signed distance to a disk
//
// List of some other 2D distances: https://www.shadertoy.com/playlist/MXdSRf
//
// and iquilezles.org/articles/distfunctions2d
float circleSD(vec2 p, float r) {
    return length(p) - r;
}


// Ellipse Signed Distance, taken from: https://www.shadertoy.com/view/4lsXDN
//
// The MIT License
// Copyright © 2013 Inigo Quilez
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions: The above copyright
// notice and this permission notice shall be included in all copies or
// substantial portions of the Software. THE SOFTWARE IS PROVIDED "AS IS",
// WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
// TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
// CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
// SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
// https://www.youtube.com/c/InigoQuilez
// https://iquilezles.org
// Analytical distance to an 2D ellipse, which is more
// complicated than it seems. It ends up being a quartic
// equation, which can be resolved through a cubic, then
// a quadratic. Some steps through the derivation can be
// found in this article:
//
// https://iquilezles.org/articles/ellipsedist
//
//
// Ellipse distances related shaders:
//
// Analytical     : https://www.shadertoy.com/view/4sS3zz
// Newton Trig    : https://www.shadertoy.com/view/4lsXDN
// Newton No-Trig : https://www.shadertoy.com/view/tttfzr
// ?????????????? : https://www.shadertoy.com/view/tt3yz7
// List of some other 2D distances: https://www.shadertoy.com/playlist/MXdSRf
//
// and iquilezles.org/articles/distfunctions2d
float ellipseSD(vec2 p, float width, float height) {
    vec2 ab = vec2(width / 2.0, height / 2.0);
    p = abs(p); // symmetry

    // find root with Newton solver
    vec2 q = ab * (p - ab);
    float w = (q.x < q.y) ? 1.570796327 : 0.0;
    for (int i = 0; i < 5; i++) {
        vec2 cs = vec2(cos(w), sin(w));
        vec2 u = ab * vec2( cs.x,  cs.y);
        vec2 v = ab * vec2(-cs.y,  cs.x);
        w = w + dot(p - u, v) / (dot(p - u, u) + dot(v, v));
    }

    // compute final point and distance
    float d = length(p - ab * vec2(cos(w), sin(w)));
    // return signed distance
    return (dot(p / ab, p / ab) > 1.0) ? d : -d;
}

void main()
{
    float sd;
    vec2  p = vPosition - uCenter;

    if (uWidth == uHeight) {
        sd = circleSD(p, uWidth / 2.0);
    } else {
        sd = ellipseSD(p, uWidth, uHeight);
    }

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
