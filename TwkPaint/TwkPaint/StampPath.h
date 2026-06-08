// Arc-length stamp placement for paint strokes.
//
// Distributes stamp instances at regular arc-length intervals along a stroke
// path. Each instance carries position, radius, opacity, angle, and squish
// values for GPU rendering. Supports spacing jitter and rotation-to-stroke-
// direction. Output is purely geometric: no image buffers or rasterization.
//
// SPDX-License-Identifier: Apache-2.0
//

#pragma once

#include <TwkMath/Vec2.h>
#include <TwkPaint/Smoother.h>
#include <memory>

namespace TwkPaint {

// ── BrushParams ───────────────────────────────────────────────────────────────
//
// Configuration for a single stamp brush stroke.
// Per-point overrides (pressure radius, tilt angle, etc.) are passed to
// add_point() directly; these fields define the base/default values.
//
struct BrushParams
{
    float radius  = 0.5f; // base stamp radius (coordinate-system agnostic)
    float opacity = 1.0f; // base stamp opacity [0..1]
    float angle   = 0.0f; // base stamp angle in degrees
    float squish  = 1.0f; // aspect ratio [0..1], 1.0 = circle

    float spacing       = 0.0f; // inter-stamp distance; 0 = use proportional default (radius * 0.5)
    float spacingBias   = 1.0f; // multiplier on default inter-stamp spacing
    float spacingJitter = 0.0f; // random spacing variation [0..1]
    float opacityJitter = 0.0f; // random opacity variation [0..1]
    float radiusJitter  = 0.0f; // random radius variation [0..1]
    float rotationJitter = 0.0f;  // random angle added per stamp (degrees)
    bool rotateToStroke  = false; // align stamp to stroke direction
};

// ── StampInstance ─────────────────────────────────────────────────────────────
//
// One placed stamp — output of StampPath::next().
// All values are in stroke (canvas) space.
//
struct StampInstance
{
    TwkMath::Vec2f pos;
    float radius;
    float opacity;
    float angle;  // degrees, clockwise from up
    float squish; // aspect ratio [0..1]
};

// ── StampPath ─────────────────────────────────────────────────────────────────
//
// Produces a stream of StampInstance placements along a stroke path.
//
// Usage:
//   StampPath placer(params);
//   for each pointer event:
//       placer.add_point(pos);               // or with per-point overrides
//       StampInstance s;
//       while (placer.next(s)) { render(s); }
//
// Per-point overrides (pass a negative value to use the BrushParams default):
//   placer.add_point(pos, radius, opacity, angle, squish);
//
class StampPath
{
  public:
    explicit StampPath(const BrushParams& params = BrushParams{});

    // Feed a new raw input point (post-input-smoother position).
    // Optional per-point overrides; pass < 0 to use BrushParams default.
    void add_point(const TwkMath::Vec2f& pt, float radius = -1.f, float opacity = -1.f,
                   float angle = -1.f, float squish = -1.f);

    // Drain the next stamp placement. Returns false when no more stamps
    // are available for the points fed so far.
    bool next(StampInstance& out);

    // Reset to initial state, optionally with new params.
    void reset(const BrushParams& params = BrushParams{});

  private:
    float default_spacing_(float radius, float squish) const;

    BrushParams params_;
    std::unique_ptr<FltInterpolate2D> interp_;

    // Per-segment interpolation state
    float from_radius_, to_radius_;
    float from_opacity_, to_opacity_;
    float from_angle_, to_angle_;
    float from_squish_, to_squish_;
    bool radius_changing_;
    bool opacity_changing_;
    bool angle_changing_;
    bool squish_changing_;

    float frac_;       // fractional position along current segment [0..1]
    float dist_;       // inter-stamp distance (same units as input coordinates)
    float base_angle_; // stroke base angle for rotate-to-stroke
    int points_seen_;
};

} // namespace TwkPaint
