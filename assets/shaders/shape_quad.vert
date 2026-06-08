// Shape bounding-box quad vertex shader — shared source for RV and CR.
//
// Prepend compat_gl21.glsl (RV / OpenGL 2.1) or compat_webgl2.glsl
// (CR / WebGL 2) before compiling.  Those headers define IN_VERT, OUT_VERT
// and the GLSL version so this file contains no version-specific syntax.
//
// Each shape is rendered as a screen-aligned bounding-box quad.  The vertex
// shader applies a 2-D affine transform (mat3, homogeneous) to map from the
// shape's local coordinate space into clip space.  A second mat3, uClip,
// carries an optional crop/scissor transform (pass mat3(1.0) to disable).
//
// Uniforms:
//   uTransform — 3×3 column-major matrix: local → clip space
//   uClip      — 3×3 column-major matrix: additional clip-space crop
//                (pass mat3(1.0) for no-op)
//
// Attributes:
//   aPosition  — 2-D vertex position in local (shape) coordinates
//
// Varyings (out):
//   vPosition  — the same local-space position, passed through to the
//                fragment shader so the SDF can be evaluated per-fragment
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

uniform mat3 uTransform;
uniform mat3 uClip;

IN_VERT  vec2 aPosition;
OUT_VERT vec2 vPosition;

void main()
{
    // Pass local position to the fragment shader for SDF evaluation.
    vPosition = aPosition;

    // Transform to clip space via the two mat3s (homogeneous 2-D).
    vec3 clipPos = uClip * uTransform * vec3(aPosition, 1.0);
    gl_Position  = vec4(clipPos.xy, 0.0, 1.0);
}
