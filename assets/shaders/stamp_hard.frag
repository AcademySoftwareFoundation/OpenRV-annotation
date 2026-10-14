// Stamp brush fragment shader — shared source for RV and CR.
//
// Prepend compat_gl21.glsl (RV / OpenGL 2.1) or compat_webgl2.glsl
// (CR / WebGL2) before compiling. Those headers define IN, FRAG_COLOR,
// and SAMPLE2D so this file contains no version-specific syntax.
//
// Input:
//   brushTip     — grayscale brush-tip texture (GL_LUMINANCE / RED).
//                  Luminance drives alpha; shape is fully defined by the PNG.
//   uniformColor — RGBA stroke colour. RGB sets the colour, A sets opacity.
//   TexCoord0    — UV coordinates [0,1] from the stamp quad vertex shader.
//
// Edge AA:
//   fwidth(tip) gives the screen-space rate-of-change of the sampled alpha.
//   smoothstep(0, fw, tip) maps that into a sub-pixel smooth transition at
//   the stamp boundary — wide enough to cover one pixel, no wider.
//   This matches the technique already used in RV's ReplaceFrag.glsl.
//
// SPDX-License-Identifier: Apache-2.0

uniform sampler2D brushTip;
uniform vec4      uniformColor;
IN vec2 TexCoord0;

void main()
{
    float tip   = SAMPLE2D(brushTip, TexCoord0).r;
    float fw    = max(fwidth(tip), 0.001);
    float alpha = smoothstep(0.0, fw, tip);
    FRAG_COLOR  = vec4(uniformColor.rgb, alpha * uniformColor.a);
}
