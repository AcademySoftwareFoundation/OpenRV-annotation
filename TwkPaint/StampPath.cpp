// Arc-length stamp placement for paint strokes.
//
// SPDX-License-Identifier: Apache-2.0
//

#include <TwkPaint/StampPath.h>
#include <cassert>
#include <cmath>
#include <cstdlib>

namespace TwkPaint {

// ── StampPath ─────────────────────────────────────────────────────────────────

StampPath::StampPath(const BrushParams& params)
{
    reset(params);
}

void StampPath::reset(const BrushParams& params)
{
    params_           = params;
    from_radius_      = params.radius;
    to_radius_        = params.radius;
    from_opacity_     = params.opacity;
    to_opacity_       = params.opacity;
    from_angle_       = params.angle;
    to_angle_         = params.angle;
    from_squish_      = params.squish;
    to_squish_        = params.squish;
    radius_changing_  = false;
    opacity_changing_ = false;
    angle_changing_   = false;
    squish_changing_  = false;
    frac_             = 0.0f;
    dist_ =
        (params.spacing > 0.f) ? params.spacing : default_spacing_(params.radius, params.squish);
    base_angle_  = params.angle;
    points_seen_ = 0;
    interp_.reset(new FltInterpolate2D);
}

void StampPath::add_point(const TwkMath::Vec2f& pt, float radius, float opacity, float angle,
                          float squish)
{
    const float r = (radius >= 0.f) ? radius : params_.radius;
    const float o = (opacity >= 0.f) ? opacity : params_.opacity;
    const float a = (angle >= 0.f) ? angle : params_.angle;
    const float s = (squish >= 0.f) ? squish : params_.squish;

    interp_->add_point(pt);

    if (points_seen_ == 0)
    {
        from_radius_  = r;
        from_opacity_ = o;
        from_angle_   = a;
        from_squish_  = s;
        base_angle_   = a;
        dist_         = (params_.spacing > 0.f) ? params_.spacing : default_spacing_(r, s);
    }
    else
    {
        // Carry over: from_ becomes the previous to_ for continuity
        // across segments (mirrors stamp_->get_* after draw_stamps in original).
        from_radius_  = to_radius_;
        from_opacity_ = to_opacity_;
        from_angle_   = to_angle_;
        from_squish_  = to_squish_;

        to_radius_  = r;
        to_opacity_ = o;
        to_angle_   = a;
        to_squish_  = s;

        radius_changing_  = (from_radius_ != to_radius_);
        opacity_changing_ = (from_opacity_ != to_opacity_);
        angle_changing_   = (from_angle_ != to_angle_);
        squish_changing_  = (from_squish_ != to_squish_);
    }

    ++points_seen_;
}

bool StampPath::next(StampInstance& out)
{
    if (points_seen_ < 2) return false;

    float* frac_p = (radius_changing_ || opacity_changing_ || angle_changing_ || squish_changing_)
                        ? &frac_
                        : nullptr;

    TwkMath::Vec2f pt;
    if (!interp_->interpolate(dist_, pt, frac_p)) return false;

    const float f = frac_p ? frac_ : 0.0f;
    const float cur_radius =
        radius_changing_ ? from_radius_ + (to_radius_ - from_radius_) * f : from_radius_;
    const float cur_opacity =
        opacity_changing_ ? from_opacity_ + (to_opacity_ - from_opacity_) * f : from_opacity_;
    const float cur_angle =
        angle_changing_ ? from_angle_ + (to_angle_ - from_angle_) * f : from_angle_;
    const float cur_squish =
        squish_changing_ ? from_squish_ + (to_squish_ - from_squish_) * f : from_squish_;

    // Recompute inter-stamp spacing when radius or squish are varying, or
    // when spacing jitter is active
    // sequence (uses the stamp values from before this stamp's lerp update).
    if (params_.spacing <= 0.f &&
        (radius_changing_ || squish_changing_ || params_.spacingJitter > 0.0f))
        dist_ = default_spacing_(cur_radius, cur_squish);

    // ── position ──────────────────────────────────────────────────────────────
    out.pos    = pt;
    out.squish = cur_squish;

    // ── opacity jitter ────────────────────────────────────────────────────────
    out.opacity = cur_opacity;
    if (params_.opacityJitter > 0.0f)
    {
        const float r =
            static_cast<float>(((rand() & 0xff) << 8) | (rand() & 0xff)) * (1.0f / 0xffff);
        out.opacity -= r * cur_opacity * (params_.opacityJitter / 20.0f);
        if (out.opacity < 0.0f) out.opacity = 0.0f;
    }

    // ── radius jitter ─────────────────────────────────────────────────────────
    out.radius = cur_radius;
    if (params_.radiusJitter > 0.0f)
    {
        const float r =
            static_cast<float>(((rand() & 0xff) << 8) | (rand() & 0xff)) * (1.0f / 0xffff);
        out.radius -= r * cur_radius * (params_.radiusJitter / 20.0f);
        if (out.radius < 0.125f) out.radius = 0.125f;
    }

    // ── angle: rotate-to-stroke + jitter ─────────────────────────────────────
    float angle = base_angle_ + cur_angle;

    if (params_.rotateToStroke)
    {
        float dx, dy;
        if (interp_->dir(dx, dy) && (dx != 0.0f || dy != 0.0f))
        {
            const float stroke_angle = std::atan2(dy, dx) * (180.0f / 3.14159265f) + 270.0f;
            angle                    = stroke_angle + base_angle_;
        }
    }

    if (params_.rotationJitter > 0.0f)
    {
        const float r = static_cast<float>(rand()) * (2.0f * params_.rotationJitter) *
                            static_cast<float>(1.0 / RAND_MAX) -
                        params_.rotationJitter;
        angle += r;
    }

    out.angle = angle;

    return true;
}

// ── default_spacing_ ──────────────────────────────────────────────────────────
//
// Returns the default inter-stamp distance when BrushParams::spacing is not
// set explicitly. Proportional to radius so it is correct in any coordinate
// system (normalized, pixel, or otherwise).
//
// Spacing = radius * 0.5 * spacingBias, adjusted for squish so tightly-
// squished stamps do not overlap excessively.
//
float StampPath::default_spacing_(float radius, float squish) const
{
    static constexpr float kFraction = 0.5f;

    // Elliptical stamps: reduce effective radius by squish² so tightly-squished
    // stamps don't overlap excessively.
    const float r = (squish < 1.0f) ? radius * squish * squish : radius;

    float d = r * kFraction * params_.spacingBias;

    if (params_.spacingJitter > 0.0f)
    {
        const float rnd   = static_cast<float>(((rand() & 0xff) << 8) | (rand() & 0xff));
        const float range = params_.spacingJitter * d;
        d += rnd * range * (1.0f / 32767.0f) - range;
    }

    // Clamp to a tiny positive value so we never get an infinite loop.
    return (d > 0.0f) ? d : radius * 0.001f;
}

} // namespace TwkPaint
