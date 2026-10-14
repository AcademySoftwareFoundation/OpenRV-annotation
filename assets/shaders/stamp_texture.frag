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

    // Radius at which the solid core ends and the fade to transparent begins.
    // hardness=0 -> fade starts at centre (fully soft); hardness=1 -> fade
    // starts almost at the edge (crisp, hard-edged stamp).
    float cutoff = hardness * 0.99;
    // Width of the fade band from cutoff out to the edge, floored by dAA so
    // the band never shrinks below one pixel (avoids aliasing/stair-stepping
    // when hardness is near 1).
    float band = max(1.0 - cutoff, fwidth(dist));
    // Re-map dist into [0,1] across just the fade band: 0 = still solid,
    // 1 = fully faded out.
    float u = clamp((dist - cutoff) / band, 0.0, 1.0);

    // Gaussian falloff across the edge band (1 inside the solid core,
    // smoothly decaying to ~0 by u=1).
    float radial = exp(-0.5 * (3.5 * u) * (3.5 * u)) * step(dist, 1.0);

    // Final alpha = texture pattern x circular falloff x per-stamp opacity.
    // Colour is constant (uniformColor); only alpha varies per fragment 
    // (opacity jitter).
    float alpha = pattern * radial * Opacity;
    FRAG_COLOR  = vec4(uniformColor.rgb, alpha);
}
