// Stamp brush vertex shader — shared source for RV and CR.
//
// Prepend compat_gl21.glsl (RV) or compat_webgl2.glsl (CR) before compiling.
//
// Transforms each stamp quad vertex through the standard projection/modelview
// stack and passes UV coordinates and per-stamp opacity to the fragment
// shader. The stroke's RGB colour is constant for an entire draw call, so
// it is passed as a `uColor` uniform (see stamp.frag) rather than a
// per-vertex attribute; only opacity varies per stamp (e.g. opacity
// jitter), so only a single float needs to be duplicated across each
// stamp's vertices to work around the lack of instanced-rendering support
// (RV's OpenGL 2.1 profile doesn't support vertexAttribDivisor).
//
// SPDX-License-Identifier: Apache-2.0

uniform mat4 projMatrix;
uniform mat4 modelviewMatrix;

IN_VERT float in_Opacity;
IN_VERT vec2 in_Position;
IN_VERT vec2 in_TexCoord0;

OUT_VERT vec2 TexCoord0;
OUT_VERT float Opacity;

void main()
{
    gl_Position = projMatrix * modelviewMatrix * vec4(in_Position, 0.0, 1.0);
    TexCoord0   = in_TexCoord0;
    Opacity     = in_Opacity;
}
