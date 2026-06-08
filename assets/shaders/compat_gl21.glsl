// Compatibility header for GLSL 1.20 (OpenGL 2.1 / RV)
// Prepend this file before any shared shader source.
//
// SPDX-License-Identifier: Apache-2.0

#version 120

// Vertex shader
#define IN_VERT    attribute
#define OUT_VERT   varying

// Fragment shader
#define IN_FRAG    varying
#define IN         varying   // alias kept for backward compat
#define FRAG_COLOR gl_FragColor
#define SAMPLE2D   texture2D

