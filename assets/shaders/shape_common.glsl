// Shape shader common helpers — shared source for RV and CR.
//
// Prepend compat_gl21.glsl (RV / OpenGL 2.1) or compat_webgl2.glsl
// (CR / WebGL 2) before including this file.  Those headers define the
// GLSL version and precision — this file contains no version-specific syntax.
//
// This file is meant to be included (concatenated) into each shape fragment
// shader; it is not compiled standalone.
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

// Smooth anti-aliased colour blend at the boundary of an SDF shape.
//
// dist — signed distance value: the blend transitions from innerColor (dist < 0)
//        to outerColor (dist > 0) over a band of width 2·aa centred on dist = 0.
// aa   — half-width of the AA band, in the same units as dist.  Caller is
//        expected to compute this from fwidth() of a screen-space-linear
//        varying (typically vPosition) so it corresponds to one rendered pixel.
//
// Unlike `1 - smoothstep(0, aa, abs(dist))`, which is symmetric around the
// boundary and produces a bright ring on both sides when outer fragments are
// rasterised (i.e. a padded bounding quad), the bilateral `smoothstep(-aa, aa,
// dist)` form fades innerColor → outerColor in a single monotonic pass, which
// is correct for either a tight or a padded bounding quad.
vec4 smoothColors(vec4 innerColor, vec4 outerColor, float dist, float aa) {
    return mix(innerColor, outerColor, smoothstep(-aa, aa, dist));
}
