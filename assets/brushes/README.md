# Brushes

Brush definitions for stamp-based paint strokes. A stroke is drawn by
repeatedly stamping the brush's tip texture along the stroke path.
`catalogue.json` lists each brush's tip and parameters; placement and jitter
are implemented in `TwkPaint/StampPath.cpp`, and the stamp's look in
`../shaders/stamp_texture.frag`.

## Fields

| Field | Meaning |
| --- | --- |
| `name` | Brush identifier. |
| `tip` | Tip texture in this folder. Only the red channel is used, as a grayscale alpha mask; colour comes from the stroke. |

## Params

| Param | Meaning |
| --- | --- |
| `spacingBias` | Distance between stamps, relative to brush size: `radius × 0.5 × spacingBias`. Low = dense, smooth line; high = visible individual stamps. Ignored if an explicit `spacing` is set. |
| `spacingJitter` | Randomizes each gap by up to ±`spacingJitter` × the normal gap. Values above 1 can collapse gaps to near zero, producing clumps. |
| `opacityJitter` | Randomly makes stamps fainter, by up to `opacityJitter / 20` of their opacity. Never increases opacity. |
| `radiusJitter` | Randomly makes stamps smaller, by up to `radiusJitter / 20` of their radius. |
| `angle` | Fixed tip rotation in degrees. When `rotateToStroke` is on, it's an offset from the stroke direction. |
| `rotateToStroke` | If `true`, each stamp turns to follow the direction of the stroke, for directional tips. |
| `rotationJitter` | Adds a random rotation between −value and +value degrees to every stamp. Large values hide the repeating texture. |
| `squish` | Aspect ratio of the stamp: 1 = circle, smaller = flatter oval. Also tightens spacing by `squish²` so ovals don't leave gaps. |
| `hardness` | Edge softness, applied as a round falloff on top of the tip texture: 1 = solid disc with a crisp edge, 0 = fades out from the centre. A stroke's own `hardness` overrides it; defaults to 0.5. |
| `flow` | Opacity of each individual stamp as a percentage of the max opacity. Overlapping stamps build up toward full strength, then the whole stroke is capped at the stroke's opacity. Low flow gives a soft, layered, airbrush-like build-up. |

## Notes

- All jitter is random, so pass a `seed` in the `BrushParams` given to the
  `StampPath` constructor to make it deterministic. With a seed, the same
  stroke always gets the same stamps on every redraw and across platforms.
- With `rotateToStroke` off, `StampPath::next()` adds `angle` twice
  (`base_angle_ + cur_angle`). All brushes here use `angle: 0`, so it has no
  visible effect yet.
