// Physics-based input smoothing for paint strokes.
//
// SPDX-License-Identifier: Apache-2.0
//

#include <TwkPaint/Smoother.h>
#include <cassert>
#include <cmath>
#include <cstdlib>

namespace TwkPaint {

// ── Smooth ────────────────────────────────────────────────────────────────────

Smooth::Smooth(float mass, float drag)
    : inv_mass_(mass)
    , drag_(drag)
    , cur_vel_(0.0f)
    , cur_acc_(0.0f)
    , cur_pos_(0.0f)
    , cur_pos_inited_(0)
{
    if (inv_mass_ != 0.0f) inv_mass_ = 1.0f / inv_mass_;
}

void Smooth::smooth(float target, int& iterations, float* smoothed)
{
    if (cur_pos_inited_ && iterations > 0)
    {
        float acc = (target - cur_pos_) * inv_mass_;
        float vel = (cur_vel_ + acc) * (1.0f - drag_);

        int sum = 0;
        for (int i = 1; i <= iterations; i++)
            sum += i;

        float acc_change = (vel - cur_vel_ - iterations * cur_acc_) / sum;

        for (int i = 0; i < iterations; i++)
        {
            cur_acc_ += acc_change;
            cur_vel_ += cur_acc_;
            cur_pos_ += cur_vel_;
            smoothed[i] = cur_pos_;
        }
    }
    else
    {
        cur_pos_        = target;
        cur_pos_inited_ = 1;
        iterations      = 0;
    }
}

// ── Smooth2D ──────────────────────────────────────────────────────────────────

Smooth2D::Smooth2D(float mass, float drag, int iterations)
    : x_smoother_(mass, drag)
    , y_smoother_(mass, drag)
    , iterations_(iterations)
    , x_smoothed_(new float[iterations])
    , y_smoothed_(new float[iterations])
    , cur_sample_(0)
    , generated_samples_(0)
{}

Smooth2D::~Smooth2D()
{
    delete[] x_smoothed_;
    delete[] y_smoothed_;
}

void Smooth2D::add_point(const TwkMath::Vec2f& pt)
{
    if (generated_samples_ != iterations_) generated_samples_ = iterations_;

    x_smoother_.smooth(pt.x, generated_samples_, x_smoothed_);
    y_smoother_.smooth(pt.y, generated_samples_, y_smoothed_);

    cur_sample_ = iterations_ - generated_samples_;
}

bool Smooth2D::interpolate(TwkMath::Vec2f& pt)
{
    if (cur_sample_ < generated_samples_)
    {
        pt = TwkMath::Vec2f(x_smoothed_[cur_sample_], y_smoothed_[cur_sample_]);
        cur_sample_++;
        return true;
    }
    return false;
}

// ── FltInterpolate2D ──────────────────────────────────────────────────────────

FltInterpolate2D::FltInterpolate2D(bool turnCorners)
    : points_(0)
    , length_(0.0f)
    , inv_length_(0.0f)
    , left_(0.0f)
    , unit_dx_(0.0f)
    , unit_dy_(0.0f)
    , backup_(0.0f)
    , first_interpolation_(true)
    , turnCorners_(turnCorners)
{}

FltInterpolate2D::~FltInterpolate2D() {}

void FltInterpolate2D::add_point(const TwkMath::Vec2f& pt)
{
    points_++;

    if (points_ > 1)
    {
        if (turnCorners_)
        {
            backup_  = left_;
            current_ = to_;
        }
        from_ = current_;
        to_   = pt;

        float xdiff          = to_.x - from_.x;
        float ydiff          = to_.y - from_.y;
        float sum_of_squares = xdiff * xdiff + ydiff * ydiff;

        length_ = std::sqrt(sum_of_squares);

        if (length_ > 0.0f)
        {
            inv_length_ = 1.0f / length_;
            unit_dx_    = xdiff * inv_length_;
            unit_dy_    = ydiff * inv_length_;
        }
        else
        {
            inv_length_ = 0.0f;
            unit_dx_    = 0.0f;
            unit_dy_    = 0.0f;
        }

        if (turnCorners_)
        {
            current_.x -= backup_ * unit_dx_;
            current_.y -= backup_ * unit_dy_;
            left_ = length_ + backup_;
        }
        else
        {
            left_ = length_;
        }
    }
    else
    {
        to_      = pt;
        current_ = pt;
    }
}

bool FltInterpolate2D::interpolate(float dist, TwkMath::Vec2f& pt, float* frac)
{
    bool can_do = false;

    if (points_ > 1)
    {
        if (first_interpolation_)
        {
            can_do               = true;
            first_interpolation_ = false;
        }
        else if (dist <= left_)
        {
            if (dist < backup_) dist = backup_;
            backup_ = 0.0f;
            current_.x += dist * unit_dx_;
            current_.y += dist * unit_dy_;
            left_ -= dist;
            can_do = true;
        }

        if (can_do)
        {
            pt = current_;
            if (frac) *frac = length_ == 0.0f ? 0.0f : 1.0f - left_ * inv_length_;
        }
    }

    return can_do;
}

bool FltInterpolate2D::interpolate(TwkMath::Vec2f& pt, float* frac)
{
    bool can_do = false;

    if (points_ > 1)
    {
        float the_frac = 0.0f;

        if (first_interpolation_)
        {
            the_frac             = 0.0f;
            first_interpolation_ = false;
            can_do               = true;
        }
        else if (left_ > 0.0f)
        {
            current_ = to_;
            the_frac = 1.0f;
            left_    = 0.0f;
            can_do   = true;
        }

        if (can_do)
        {
            pt = current_;
            if (frac) *frac = the_frac;
        }
    }

    return can_do;
}

bool FltInterpolate2D::dir(float& x, float& y)
{
    if (points_ > 1 && length_ > 0.0f)
    {
        x = unit_dx_;
        y = unit_dy_;
        return true;
    }
    return false;
}

// ── SmoothInterpolate2D ───────────────────────────────────────────────────────

SmoothInterpolate2D::SmoothInterpolate2D(float mass, float drag, int iterations,
                                         unsigned int smoothLevel)
    : x_smoother_(mass, drag)
    , y_smoother_(mass, drag)
    , iterations_(iterations)
    , x_smoothed_(new float[iterations])
    , y_smoothed_(new float[iterations])
    , cur_sample_(0)
    , generated_samples_(0)
    , inv_generated_samples_(0.0f)
{
    if (smoothLevel <= 1)
        between_.reset(new FltInterpolate2D);
    else
        between_.reset(new SmoothInterpolate2D(mass, drag, iterations, smoothLevel - 1));
}

SmoothInterpolate2D::SmoothInterpolate2D(float mass, float drag, int iterations,
                                         Interpolate2D* between)
    : x_smoother_(mass, drag)
    , y_smoother_(mass, drag)
    , iterations_(iterations)
    , x_smoothed_(new float[iterations])
    , y_smoothed_(new float[iterations])
    , cur_sample_(0)
    , generated_samples_(0)
    , inv_generated_samples_(0.0f)
    , between_(between)
{
    assert(between_);
}

SmoothInterpolate2D::~SmoothInterpolate2D()
{
    delete[] x_smoothed_;
    delete[] y_smoothed_;
}

void SmoothInterpolate2D::add_point(const TwkMath::Vec2f& pt)
{
    if (generated_samples_ != iterations_)
    {
        generated_samples_ = iterations_;
        if (generated_samples_ > 0)
            inv_generated_samples_ = 1.0f / static_cast<float>(generated_samples_);
    }

    x_smoother_.smooth(pt.x, generated_samples_, x_smoothed_);
    y_smoother_.smooth(pt.y, generated_samples_, y_smoothed_);

    cur_sample_ = iterations_ - generated_samples_;
}

bool SmoothInterpolate2D::interpolate(float dist, TwkMath::Vec2f& pt, float* frac)
{
    bool can_do;

    while (!(can_do = between_->interpolate(dist, pt, frac)) && cur_sample_ < generated_samples_)
    {
        between_->add_point(TwkMath::Vec2f(x_smoothed_[cur_sample_], y_smoothed_[cur_sample_]));
        cur_sample_++;
    }

    if (can_do && frac)
    {
        *frac = (cur_sample_ - 1 + *frac) * inv_generated_samples_;
        if (*frac < 0.0f) *frac = 0.0f;
    }

    return can_do;
}

bool SmoothInterpolate2D::interpolate(TwkMath::Vec2f& pt, float* frac)
{
    bool can_do;

    while (!(can_do = between_->interpolate(pt, frac)) && cur_sample_ < generated_samples_)
    {
        between_->add_point(TwkMath::Vec2f(x_smoothed_[cur_sample_], y_smoothed_[cur_sample_]));
        cur_sample_++;
    }

    if (can_do && frac)
    {
        *frac = (cur_sample_ - 1 + *frac) * inv_generated_samples_;
        if (*frac < 0.0f) *frac = 0.0f;
    }

    return can_do;
}

bool SmoothInterpolate2D::dir(float& x, float& y)
{
    return between_->dir(x, y);
}

} // namespace TwkPaint
