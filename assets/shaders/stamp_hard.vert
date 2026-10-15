// Stamp brush vertex shader — shared source for RV and CR.
//
// Prepend compat_gl21.glsl (RV) or compat_webgl2.glsl (CR) before compiling.
//
// Transforms each stamp quad vertex through the standard projection/modelview
// stack and passes UV coordinates to the fragment shader.
//
// SPDX-License-Identifier: Apache-2.0

uniform mat4 projMatrix;
uniform mat4 modelviewMatrix;

IN_VERT vec2 in_Position;
IN_VERT vec2 in_TexCoord0;
OUT_VERT vec2 TexCoord0;

void main()
{
    gl_Position = projMatrix * modelviewMatrix * vec4(in_Position, 0.0, 1.0);
    TexCoord0   = in_TexCoord0;
}
