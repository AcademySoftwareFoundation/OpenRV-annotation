// Line (segment) SDF fragment shader — shared source for RV and CR.
//
// Prepend compat_gl21.glsl (RV / OpenGL 2.1) or compat_webgl2.glsl
// (CR / WebGL 2) before compiling, then concatenate shape_common.glsl.
// Those files define FRAG_COLOR, IN_FRAG, and smoothColors() so this file
// contains no version-specific syntax.
//
// Renders a straight line segment (rounded capsule) using a signed-distance
// field.  The bounding quad is supplied by shape_quad.vert.
//
// Uniforms:
//   uStart       — start endpoint (vec2, local coords)
//   uEnd         — end   endpoint (vec2, local coords)
//   uBorderColor — RGBA colour of the line
//   uBorderWidth — half-width of the line in local units
//
// Note: lines have no interior fill, so uInnerColor is intentionally absent.
// The SDF drives directly from the capsule boundary inward.
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

uniform vec2 uStart;
uniform vec2 uEnd;
uniform vec4 uBorderColor;
uniform float uBorderWidth;
// See rectangle.frag for the uDpr contract.
uniform float uDpr;

IN_FRAG vec2 vPosition;

// Segment Signed Distance, taken from: https://www.shadertoy.com/view/3tdSDj
//
// The MIT License
// Copyright © 2020 Inigo Quilez
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
//
// Unsigned distance from point p to the line segment [a, b].
//
// See: https://iquilezles.org/articles/distfunctions2d
float sdSegment(vec2 p, vec2 a, vec2 b)
{
    vec2 pa = p - a;
    vec2 ba = b - a;
    float h = clamp(dot(pa, ba) / dot(ba, ba), 0.0, 1.0);
    return length(pa - ba * h);
}

void main()
{
    // sdSegment returns unsigned distance from the centre line.
    // Subtract uBorderWidth to get a signed distance relative to the capsule edge.
    float sd = sdSegment(vPosition, uStart, uEnd) - uBorderWidth;

    float aa = fwidth(vPosition.x) * uDpr;
    if (sd > aa) {
        discard;
    } else {
        // Blend to transparent at the outer edge for anti-aliasing.
        FRAG_COLOR = smoothColors(uBorderColor, vec4(uBorderColor.rgb, 0.0), sd, aa);
    }
}
