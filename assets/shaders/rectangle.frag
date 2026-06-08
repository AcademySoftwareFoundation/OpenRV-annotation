// Rectangle SDF fragment shader — shared source for RV and CR.
//
// Prepend compat_gl21.glsl (RV / OpenGL 2.1) or compat_webgl2.glsl
// (CR / WebGL 2) before compiling, then concatenate shape_common.glsl.
// Those files define FRAG_COLOR, IN_FRAG, and smoothColors() so this file
// contains no version-specific syntax.
//
// Renders a (optionally rounded) rectangle using a signed-distance field.
// The bounding quad is supplied by shape_quad.vert.
//
// Uniforms:
//   uCenter      — centre of the rectangle in local (shape) coordinates
//   uWidth       — full width of the rectangle
//   uHeight      — full height of the rectangle
//   uCornerRadii — per-corner radii as vec4(topRight, bottomRight,
//                  topLeft, bottomLeft); pass vec4(0.0) for sharp corners
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

uniform vec2 uCenter;
uniform float uWidth;
uniform float uHeight;
uniform vec4  uCornerRadii;
uniform vec4  uInnerColor;
uniform vec4  uBorderColor;
uniform float uBorderWidth;
// Scale factor applied to the AA band width. Pass 1.0 when rendering
// directly to screen-resolution pixels. Pass window.devicePixelRatio
// when rendering to an offscreen buffer that is later bilinearly
// upscaled, so the AA band is wide enough to survive the
// upscale without revealing sub-pixel stair-stepping.
uniform float uDpr;

IN_FRAG vec2 vPosition;

// Rectangle Signed Distance, taken from: https://www.shadertoy.com/view/4llXD7
//
// The MIT License
// Copyright © 2015 Inigo Quilez
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
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
// FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
// THE USE OR OTHER DEALINGS IN THE SOFTWARE.
// https://www.youtube.com/c/InigoQuilez
// https://iquilezles.org
float roundRectangleSD(vec2 p, float width, float height, vec4 r) {
    vec2 b = vec2(width / 2.0, height / 2.0);
    r.xy = (p.x > 0.0) ? r.xy : r.zw;
    r.x  = (p.y > 0.0) ? r.x  : r.y;
    vec2 q = abs(p) - b + r.x;
    return min(max(q.x, q.y), 0.0) + length(max(q, 0.0)) - r.x;
}

void main()
{
    vec2  p  = vPosition - uCenter;
    float sd = roundRectangleSD(p, uWidth, uHeight, uCornerRadii);

    // Compute aa from vPosition rather than sd to avoid gradient spikes at
    // SDF seams. Multiplying by uDpr widens the AA
    // band so that after bilinear upscale from an offscreen buffer
    // still reads as a smooth edge; with uDpr = 1 it collapses to one render-pixel of AA.
    float aa = fwidth(vPosition.x) * uDpr;

    // When fading to a zero-alpha colour we keep the
    // adjacent visible colour's RGB and vary only alpha. Blending straight
    // alpha against vec4(0.0) would otherwise mix RGB toward black across
    // the AA band, producing a dark edge when composited over a bright
    // background. For the fill-to-border transition we only substitute when
    // the fill itself is fully transparent — a visible fill colour must be
    // preserved in the deep interior.
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
