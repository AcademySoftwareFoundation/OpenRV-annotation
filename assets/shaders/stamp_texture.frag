// Prepend compat_gl21.glsl (RV / OpenGL 2.1) or compat_webgl2.glsl
// (CR / WebGL2) before compiling.
//
// Renders a pattern texture (the brush tip's red channel) multiplied by a
// circular radial falloff.
//
// Input:
//   brushTip     — pattern/shape mask texture, read from the red channel.
//   uColor       — RGBA stroke colour, constant for the whole draw call.
//                  RGB sets the colour; A is unused since final alpha comes
//                  from the per-vertex Opacity (see stamp.vert).
//   TexCoord0    — UV coordinates [0,1] from the stamp quad vertex shader.
//   Opacity      — per-stamp opacity (for opacity jitter), duplicated
//                  across each stamp's vertices in stamp.vert.
//   hardness     — circular falloff sharpness, 0.0 (fully soft, Gaussian
//                  fade from the centre) to 1.0 (fully hard, crisp edge).
//                  A solid core sized by hardness, with a Gaussian fade
//                  from there out to the stamp's edge.
//
// SPDX-License-Identifier: Apache-2.0

uniform sampler2D brushTip;
uniform float hardness;
uniform vec4 uniformColor;

IN_FRAG vec2 TexCoord0;
IN_FRAG float Opacity;

void main()
{
    // Grayscale brush pattern (texture) at this fragment, used as an
    // extra per-pixel alpha mask so the stamp isn't a uniform blob.
    float pattern = SAMPLE2D(brushTip, TexCoord0).r;

    // Calculate distance from the centre of the stamp.
    // 0 at centre, 1 at the stamp's circular edge.
    vec2 m = TexCoord0 - vec2(0.5);
    float dist = length(m) * 2.0;

    // Screen-space size of one pixel in dist units.
    float aa = fwidth(dist);

    // Width of the fade band, ending exactly at the stamp's edge (dist = 1).
    // hardness=0 -> band spans the whole radius (fully soft); hardness=1 ->
    // band is nearly zero (crisp edge). Floored to ~1.5 px so small stamps
    // keep a resolvable fade instead of a sub-pixel, aliased edge.
    float band = max(1.0 - hardness * 0.99, 1.5 * aa);
    // Radius at which the solid core ends and the fade begins. Derived from
    // band (rather than hardness) so widening the band moves it inward
    // instead of pushing the fade past the edge.
    float cutoff = 1.0 - band;
    // Re-map dist into [0,1] across just the fade band: 0 = still solid,
    // 1 = fully faded out.
    float u = clamp((dist - cutoff) / band, 0.0, 1.0);

    // Anti-aliased circular clip at dist = 1 (a hard step() here aliases
    // whenever the Gaussian hasn't fully decayed by the edge).
    float edge = 1.0 - smoothstep(1.0 - aa, 1.0, dist);

    // Gaussian falloff across the edge band (1 inside the solid core,
    // smoothly decaying to ~0 by u=1).
    float radial = exp(-0.5 * (3.5 * u) * (3.5 * u)) * edge;

    // Final alpha = texture pattern x circular falloff x per-stamp opacity.
    // Colour is constant (uniformColor); only alpha varies per fragment 
    // (opacity jitter).
    float alpha = pattern * radial * Opacity;
    FRAG_COLOR  = vec4(uniformColor.rgb, alpha);
}
