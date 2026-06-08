// Compatibility header for GLSL ES 3.00 (WebGL2 / CR)
// Prepend this file before any shared shader source.
//
// SPDX-License-Identifier: Apache-2.0

#version 300 es
precision mediump float;

// Vertex shader
#define IN_VERT    in
#define OUT_VERT   out

// Fragment shader
#define IN_FRAG    in
#define IN         in    // alias kept for backward compat
#define SAMPLE2D   texture

out vec4 fragColor;
#define FRAG_COLOR fragColor
