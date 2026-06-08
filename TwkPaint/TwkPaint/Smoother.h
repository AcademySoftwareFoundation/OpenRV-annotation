// Physics-based input smoothing for paint strokes.
//
// Uses a mass/drag simulation: each input point is the target position for
// a simulated particle. The particle chases the target with configurable
// mass and drag, producing smoothed output coordinates. Multiple nesting
// levels can be applied for progressively smoother results.
//
// SPDX-License-Identifier: Apache-2.0
//

#pragma once

#include <TwkMath/Vec2.h>
#include <memory>

namespace TwkPaint {

// ── Smooth ────────────────────────────────────────────────────────────────────
//
// One-dimensional physics-based smoother.
// Models the input point as a target that a particle of given mass and drag
// chases. smooth() advances the simulation and writes `iterations` output
// samples into the caller-supplied array.
//
class Smooth
{
  public:
    Smooth(float mass, float drag);
    ~Smooth() {}

    void smooth(float target, int& iterations, float* smoothed);

  protected:
    Smooth(const Smooth&);
    Smooth& operator=(const Smooth&);

  private:
    float inv_mass_;
    float drag_;
    float cur_vel_;
    float cur_acc_;
    float cur_pos_;
    int cur_pos_inited_;
};

// ── Smooth2D ──────────────────────────────────────────────────────────────────
//
// Two-dimensional physics-based smoother.
//
// Usage:
//   Smooth2D smoother;
//   smoother.add_point(raw_pt);
//   TwkMath::Vec2f out;
//   while (smoother.interpolate(out)) { /* use out */ }
//
class Smooth2D
{
  public:
    explicit Smooth2D(float mass = 0.9f, float drag = 0.921f, int iterations = 6);
    ~Smooth2D();

    void add_point(const TwkMath::Vec2f& pt);
    bool interpolate(TwkMath::Vec2f& next_pt);

  protected:
    Smooth2D(const Smooth2D&);
    Smooth2D& operator=(const Smooth2D&);

  private:
    Smooth x_smoother_;
    Smooth y_smoother_;
    int iterations_;
    float* x_smoothed_;
    float* y_smoothed_;
    int cur_sample_;
    int generated_samples_;
};

// ── Interpolate2D ─────────────────────────────────────────────────────────────
//
// Abstract interface for 2D interpolators.
// Consumers add raw points and then drain interpolated output points by calling
// one of the interpolate() overloads.
//
class Interpolate2D
{
  public:
    Interpolate2D() {}
    virtual ~Interpolate2D() {}

    virtual void add_point(const TwkMath::Vec2f& pt) = 0;

    // Interpolate for a given distance along the path.
    // frac (if non-null) is set to the fractional distance between the last
    // two added points. Returns false when no more points are available.
    virtual bool interpolate(float dist, TwkMath::Vec2f& next_pt, float* frac = nullptr) = 0;

    // Interpolate for an implementation-specific distance.
    virtual bool interpolate(TwkMath::Vec2f& next_pt, float* frac = nullptr) = 0;

    // Returns the current direction as a normalized vector.
    virtual bool dir(float& x, float& y) = 0;

  protected:
    Interpolate2D(const Interpolate2D&) {}
    Interpolate2D& operator=(const Interpolate2D&) { return *this; }
};

// ── FltInterpolate2D ──────────────────────────────────────────────────────────
//
// Linear interpolator — walks along straight line segments between added points.
//
class FltInterpolate2D : public Interpolate2D
{
  public:
    explicit FltInterpolate2D(bool turnCorners = true);
    ~FltInterpolate2D() override;

    void add_point(const TwkMath::Vec2f& pt) override;
    bool interpolate(float dist, TwkMath::Vec2f& next_pt, float* frac = nullptr) override;
    bool interpolate(TwkMath::Vec2f& next_pt, float* frac = nullptr) override;
    bool dir(float& x, float& y) override;

  protected:
    FltInterpolate2D(const FltInterpolate2D&);
    FltInterpolate2D& operator=(const FltInterpolate2D&);

  private:
    int points_;
    TwkMath::Vec2f from_, to_, current_;
    float length_;
    float inv_length_;
    float left_;
    float unit_dx_;
    float unit_dy_;
    float backup_;
    bool first_interpolation_;
    bool turnCorners_;
};

// ── SmoothInterpolate2D ───────────────────────────────────────────────────────
//
// Physics-based smoothing interpolator.
// Smooths raw input points via Smooth, then feeds the smoothed points to a
// sub-interpolator (FltInterpolate2D by default). Can be nested (smoothLevel > 1)
// for multi-pass smoothing.
//
class SmoothInterpolate2D : public Interpolate2D
{
  public:
    // smoothLevel controls nesting depth of sub-interpolators:
    //   1 → one layer of smoothing + linear interpolation
    //   2 → two layers of smoothing (default)
    explicit SmoothInterpolate2D(float mass = 0.9f, float drag = 0.921f, int iterations = 6,
                                 unsigned int smoothLevel = 2);

    // Takes ownership of `between`.
    SmoothInterpolate2D(float mass, float drag, int iterations, Interpolate2D* between);
    ~SmoothInterpolate2D() override;

    void add_point(const TwkMath::Vec2f& pt) override;
    bool interpolate(float dist, TwkMath::Vec2f& next_pt, float* frac = nullptr) override;
    bool interpolate(TwkMath::Vec2f& next_pt, float* frac = nullptr) override;
    bool dir(float& x, float& y) override;

  protected:
    SmoothInterpolate2D(const SmoothInterpolate2D&);
    SmoothInterpolate2D& operator=(const SmoothInterpolate2D&);

  private:
    Smooth x_smoother_;
    Smooth y_smoother_;
    int iterations_;
    float* x_smoothed_;
    float* y_smoothed_;
    int cur_sample_;
    int generated_samples_;
    float inv_generated_samples_;
    std::unique_ptr<Interpolate2D> between_;
};

} // namespace TwkPaint
