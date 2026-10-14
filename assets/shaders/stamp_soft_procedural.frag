// Soft (Gaussian) stamp fragment shader — shared source for RV and CR.
//
// Prepend compat_gl21.glsl (RV) or compat_webgl2.glsl (CR) before compiling.
//
// Procedural Gaussian falloff — no texture needed. The alpha at each pixel
// is exp(-r²) with a small linear correction term to ensure it reaches zero
// cleanly at the stamp boundary (r = 1.0, i.e. TexCoord0 = [0,1] quad edge).
//
// Derivation:
//   ratio = 2 * distance_from_centre / radius  (radius = 0.5 in UV space)
//   alpha = exp(-ratio²) - exp(-4) * ratio * 0.5   for ratio <= 2
//         = 0                                        for ratio > 2
//   The correction term (exp(-4) * ratio * 0.5) tapers the tail to zero at
//   ratio = 2 (the stamp boundary), avoiding a hard cut-off.
//
// SPDX-License-Identifier: Apache-2.0

uniform vec4 uniformColor;
IN_FRAG vec2 TexCoord0;

void main()
{
    vec2  m     = TexCoord0 - vec2(0.5, 0.5);
    float mag   = sqrt(m.x * m.x + m.y * m.y);
    float bot   = 0.01831563888; // exp(-4.0)
    float ratio = mag * 2.0 / 0.5;

    float a = (ratio <= 2.0) ? exp(-ratio * ratio) - bot * ratio * 0.5 : 0.0;
    FRAG_COLOR = vec4(uniformColor.rgb, a * uniformColor.a);
}
