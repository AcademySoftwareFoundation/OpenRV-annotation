#!/usr/bin/env python3
"""
Generate grayscale brush tip alpha maps for the annotation platform.

Outputs PNG files to assets/brushes/. Each file is an 8-bit grayscale image
where pixel intensity represents brush opacity (255 = fully opaque centre,
0 = fully transparent edge). The stamp fragment shader samples these as the
alpha channel of each stamp.

Usage:
    python3 scripts/gen_brush_tips.py [--size 128] [--outdir assets/brushes]

Requirements:
    pip install numpy pillow
"""

import argparse
import math
import os
import random

import numpy as np
from PIL import Image

# ---------------------------------------------------------------------------
# Brush generators
# Each returns a float32 numpy array shaped (size, size) with values in [0,1].
# ---------------------------------------------------------------------------


def _radial_coords(size):
    """Return normalised (dx, dy, r) grids centred on the image, r in [0,1]."""
    half = size / 2.0
    ax = (np.arange(size) - half + 0.5) / half  # [-1, 1]
    x, y = np.meshgrid(ax, ax)
    r = np.sqrt(x * x + y * y)
    return x, y, r


def marker(size, hardness=0.85, noise_strength=0.04, seed=42):
    """
    Hard-edged marker tip.

    A near-flat interior fades to transparent over a narrow band at the edge.
    Subtle edge noise breaks the perfect-circle silhouette for a felt-tip feel.
    """
    rng = np.random.default_rng(seed)
    _, _, r = _radial_coords(size)

    # Smooth step from opaque (r < hardness) to transparent (r > 1)
    band = 1.0 - hardness
    t = np.clip((r - hardness) / band, 0.0, 1.0)
    alpha = 1.0 - t * t * (3.0 - 2.0 * t)  # smoothstep

    # Edge noise: perturb the radius threshold slightly per-pixel
    if noise_strength > 0.0:
        noise = rng.uniform(-noise_strength, noise_strength, size=(size, size))
        noisy_r = np.clip(r + noise, 0.0, None)
        t2 = np.clip((noisy_r - hardness) / band, 0.0, 1.0)
        alpha_noisy = 1.0 - t2 * t2 * (3.0 - 2.0 * t2)
        alpha = np.maximum(alpha, alpha_noisy) * 0.5 + alpha * 0.5

    return np.clip(alpha, 0.0, 1.0)


def airbrush(size, sigma=0.38):
    """
    Soft Gaussian airbrush tip.

    Pure radial Gaussian — very soft centre with a long falloff. sigma is
    expressed as a fraction of the tip radius (0.38 ≈ moderate spray density).
    """
    _, _, r = _radial_coords(size)
    alpha = np.exp(-(r * r) / (2.0 * sigma * sigma))
    # Normalise so the peak is exactly 1.0
    alpha /= alpha.max()
    return np.clip(alpha, 0.0, 1.0)


def soft_marker(size, hardness=0.55, sigma_blend=0.22):
    """
    Intermediate between marker and airbrush.

    A Gaussian core blended with a softer smoothstep edge — rounder than the
    marker but more opaque in the centre than a pure airbrush.
    """
    _, _, r = _radial_coords(size)

    # Gaussian core
    gauss = np.exp(-(r * r) / (2.0 * sigma_blend * sigma_blend))

    # Smoothstep falloff
    band = 1.0 - hardness
    t = np.clip((r - hardness) / band, 0.0, 1.0)
    step = 1.0 - t * t * (3.0 - 2.0 * t)

    # Blend: keep whichever is larger so the centre stays full
    alpha = np.maximum(gauss, step) * 0.5 + np.minimum(gauss, step) * 0.5
    alpha /= alpha.max()
    return np.clip(alpha, 0.0, 1.0)


def chalk(size, hardness=0.70, grain_scale=6, grain_strength=0.45, seed=7):
    """
    Chalk / pastel tip.

    A roughly hard-edged circle with internal grain texture that breaks up
    the fill, giving a matte, dusty appearance.
    """
    rng = np.random.default_rng(seed)
    _, _, r = _radial_coords(size)

    # Base shape: hard circle
    band = 1.0 - hardness
    t = np.clip((r - hardness) / band, 0.0, 1.0)
    base = 1.0 - t * t * (3.0 - 2.0 * t)

    # Grain: tiled Gaussian blobs at a coarser resolution
    grain_size = max(4, size // grain_scale)
    raw_grain = rng.standard_normal((grain_size, grain_size)).astype(np.float32)
    # Upscale with bilinear interpolation via PIL
    grain_img = Image.fromarray(
        np.clip((raw_grain * 0.5 + 0.5) * 255, 0, 255).astype(np.uint8)
    ).resize((size, size), Image.BILINEAR)
    grain = np.array(grain_img, dtype=np.float32) / 255.0

    # Modulate base by grain and re-clamp inside the circle
    alpha = base * (1.0 - grain_strength + grain * grain_strength)
    alpha *= (r < 1.0).astype(np.float32)
    alpha /= alpha.max()
    return np.clip(alpha, 0.0, 1.0)


def pencil(
    size,
    hardness=0.60,
    grain_scale=4,
    grain_strength=0.60,
    streak_count=14,
    streak_strength=0.25,
    seed=13,
):
    """
    Pencil / graphite tip.

    Elliptical shape (wider than tall) with heavy grain and subtle parallel
    streaks along the brush direction to simulate graphite drag.
    """
    rng = np.random.default_rng(seed)
    half = size / 2.0
    ax = (np.arange(size) - half + 0.5) / half
    x, y = np.meshgrid(ax, ax)

    # Slightly elliptical (wider x, narrower y)
    r = np.sqrt((x / 1.15) ** 2 + (y / 0.88) ** 2)

    band = 1.0 - hardness
    t = np.clip((r - hardness) / band, 0.0, 1.0)
    base = 1.0 - t * t * (3.0 - 2.0 * t)

    # Grain (same as chalk but stronger)
    grain_size = max(4, size // grain_scale)
    raw_grain = rng.standard_normal((grain_size, grain_size)).astype(np.float32)
    grain_img = Image.fromarray(
        np.clip((raw_grain * 0.5 + 0.5) * 255, 0, 255).astype(np.uint8)
    ).resize((size, size), Image.BILINEAR)
    grain = np.array(grain_img, dtype=np.float32) / 255.0

    # Horizontal streaks (simulate graphite fibres along the brush axis)
    streaks = np.zeros((size, size), dtype=np.float32)
    for _ in range(streak_count):
        cy = rng.uniform(-1.0, 1.0)
        width = rng.uniform(0.01, 0.04)
        streak_r = np.exp(-((y - cy) ** 2) / (2.0 * width**2))
        streaks = np.maximum(streaks, streak_r)

    alpha = base * (1.0 - grain_strength + grain * grain_strength)
    alpha = alpha * (1.0 - streak_strength) + alpha * streaks * streak_strength
    alpha *= (r < 1.0).astype(np.float32)
    if alpha.max() > 0:
        alpha /= alpha.max()
    return np.clip(alpha, 0.0, 1.0)


# ---------------------------------------------------------------------------
# Catalogue
# ---------------------------------------------------------------------------

BRUSHES = [
    # (filename_stem, generator_fn, kwargs)
    ("marker", marker, {}),
    ("airbrush", airbrush, {}),
    ("soft_marker", soft_marker, {}),
    ("chalk", chalk, {}),
    ("pencil", pencil, {}),
]


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------


def generate_all(size, outdir):
    os.makedirs(outdir, exist_ok=True)
    for stem, fn, kwargs in BRUSHES:
        alpha = fn(size, **kwargs)
        pixels = (np.clip(alpha, 0.0, 1.0) * 255.0 + 0.5).astype(np.uint8)
        img = Image.fromarray(pixels, mode="L")
        path = os.path.join(outdir, f"{stem}_{size}.png")
        img.save(path, optimize=True)
        print(f"  {path}  ({pixels.shape[0]}×{pixels.shape[1]})")


def main():
    parser = argparse.ArgumentParser(description="Generate brush tip alpha maps")
    parser.add_argument(
        "--size", type=int, default=128, help="Tip resolution in pixels (default: 128)"
    )
    parser.add_argument(
        "--outdir",
        default="assets/brushes",
        help="Output directory (default: assets/brushes)",
    )
    args = parser.parse_args()

    print(
        f"Generating {len(BRUSHES)} brush tips at {args.size}×{args.size}px → {args.outdir}/"
    )
    generate_all(args.size, args.outdir)
    print("Done.")


if __name__ == "__main__":
    main()
