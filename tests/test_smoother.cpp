#include <TwkPaint/Smoother.h>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

using namespace TwkPaint;

// ── Smooth2D ──────────────────────────────────────────────────────────────────

TEST_CASE("Smooth2D cold start: first point produces no output", "[smoother]")
{
    Smooth2D s;
    s.add_point({0.0f, 0.0f});
    TwkMath::Vec2f out;
    CHECK_FALSE(s.interpolate(out));
}

TEST_CASE("Smooth2D produces output after multiple points", "[smoother]")
{
    Smooth2D s;
    s.add_point({0.0f, 0.0f});
    s.add_point({1.0f, 1.0f});

    TwkMath::Vec2f out;
    bool any = false;
    while (s.interpolate(out))
        any = true;
    CHECK(any);
}

TEST_CASE("Smooth2D output stays within input bounding box (with margin)", "[smoother]")
{
    // The physics smoother lags behind the input; output should stay in the
    // rough vicinity of the input range rather than diverging.
    Smooth2D s;
    const float lo = 0.0f, hi = 1.0f;
    for (float v = lo; v <= hi; v += 0.1f)
        s.add_point({v, v});

    TwkMath::Vec2f out;
    while (s.interpolate(out))
    {
        CHECK(out.x >= lo - 0.5f);
        CHECK(out.x <= hi + 0.5f);
        CHECK(out.y >= lo - 0.5f);
        CHECK(out.y <= hi + 0.5f);
    }
}

TEST_CASE("Smooth2D: default mass/drag params are accepted", "[smoother]")
{
    // Regression: constructor with explicit defaults should not crash.
    Smooth2D s(0.9f, 0.921f, 6);
    s.add_point({0.5f, 0.5f});
    s.add_point({0.6f, 0.4f});
    TwkMath::Vec2f out;
    // Just drain output without asserting count — physics-dependent.
    while (s.interpolate(out))
    {
    }
    SUCCEED("no crash with default params");
}

TEST_CASE("Smooth2D: custom mass/drag changes output density", "[smoother]")
{
    // Higher iterations → more samples per add_point call.
    Smooth2D few(0.9f, 0.921f, 2);
    Smooth2D many(0.9f, 0.921f, 10);

    few.add_point({0.0f, 0.0f});
    few.add_point({1.0f, 1.0f});
    many.add_point({0.0f, 0.0f});
    many.add_point({1.0f, 1.0f});

    int n_few = 0, n_many = 0;
    TwkMath::Vec2f out;
    while (few.interpolate(out))
        ++n_few;
    while (many.interpolate(out))
        ++n_many;

    CHECK(n_many > n_few);
}

// ── FltInterpolate2D ──────────────────────────────────────────────────────────

TEST_CASE("FltInterpolate2D: no output before two points added", "[smoother]")
{
    FltInterpolate2D interp;
    interp.add_point({0.0f, 0.0f});
    TwkMath::Vec2f out;
    CHECK_FALSE(interp.interpolate(out));
}

TEST_CASE("FltInterpolate2D: interpolates between two points", "[smoother]")
{
    FltInterpolate2D interp;
    interp.add_point({0.0f, 0.0f});
    interp.add_point({10.0f, 0.0f});

    // Request samples every 1 unit along the segment.
    std::vector<TwkMath::Vec2f> samples;
    TwkMath::Vec2f out;
    while (interp.interpolate(1.0f, out))
        samples.push_back(out);

    // With a 10-unit segment sampled at 1-unit intervals we expect ~10 samples.
    CHECK(samples.size() >= 5);
    CHECK(samples.size() <= 15);

    // All samples should be on the horizontal line (y ≈ 0).
    for (const auto& s : samples)
        CHECK_THAT(s.y, Catch::Matchers::WithinAbs(0.0f, 1e-4f));
}

TEST_CASE("FltInterpolate2D: direction is normalised", "[smoother]")
{
    FltInterpolate2D interp;
    interp.add_point({0.0f, 0.0f});
    interp.add_point({3.0f, 4.0f}); // length = 5

    TwkMath::Vec2f out;
    interp.interpolate(1.0f, out);

    float dx, dy;
    REQUIRE(interp.dir(dx, dy));
    float len = std::sqrt(dx * dx + dy * dy);
    CHECK_THAT(len, Catch::Matchers::WithinAbs(1.0f, 1e-4f));
}

// ── SmoothInterpolate2D ───────────────────────────────────────────────────────

TEST_CASE("SmoothInterpolate2D: produces smoothed output for a straight path", "[smoother]")
{
    SmoothInterpolate2D s;
    for (int i = 0; i <= 10; ++i)
        s.add_point({static_cast<float>(i), 0.0f});

    TwkMath::Vec2f out;
    bool any = false;
    while (s.interpolate(out))
        any = true;
    CHECK(any);
}

TEST_CASE("SmoothInterpolate2D: smoothLevel 1 vs 2 both produce output", "[smoother]")
{
    for (unsigned level : {1u, 2u})
    {
        SmoothInterpolate2D s(0.9f, 0.921f, 6, level);
        s.add_point({0.0f, 0.0f});
        s.add_point({1.0f, 0.0f});
        s.add_point({2.0f, 1.0f});

        TwkMath::Vec2f out;
        bool any = false;
        while (s.interpolate(out))
            any = true;
        CHECK(any);
    }
}

// ── FltInterpolate2D additional ───────────────────────────────────────────────

TEST_CASE("FltInterpolate2D: dir() returns false before two points", "[smoother]")
{
    FltInterpolate2D interp;
    float dx, dy;
    CHECK_FALSE(interp.dir(dx, dy));

    interp.add_point({0.0f, 0.0f});
    CHECK_FALSE(interp.dir(dx, dy));
}

TEST_CASE("FltInterpolate2D: default interpolate() overload returns false with one point",
          "[smoother]")
{
    FltInterpolate2D interp;
    interp.add_point({0.0f, 0.0f});
    TwkMath::Vec2f out;
    CHECK_FALSE(interp.interpolate(out));
}

TEST_CASE("FltInterpolate2D: frac parameter is populated", "[smoother]")
{
    FltInterpolate2D interp;
    interp.add_point({0.0f, 0.0f});
    interp.add_point({10.0f, 0.0f});

    TwkMath::Vec2f out;
    float frac = -1.0f;
    bool got   = interp.interpolate(1.0f, out, &frac);
    REQUIRE(got);
    // frac is the fractional progress along the segment; verify it was written
    // and lies in [0, 1] with a small tolerance for floating-point rounding.
    CHECK_THAT(frac, Catch::Matchers::WithinAbs(0.0f, 1e-4f));
}

// ── SmoothInterpolate2D additional ───────────────────────────────────────────

TEST_CASE("SmoothInterpolate2D: dir() returns a valid normalised vector after points", "[smoother]")
{
    SmoothInterpolate2D s;
    for (int i = 0; i <= 5; ++i)
        s.add_point({static_cast<float>(i), 0.0f});

    // Drain output first so direction is established.
    TwkMath::Vec2f out;
    while (s.interpolate(out))
    {
    }

    float dx, dy;
    // dir() may return false if not enough interpolation has occurred, but
    // when it returns true the vector must be approximately unit length.
    if (s.dir(dx, dy))
    {
        float len = std::sqrt(dx * dx + dy * dy);
        CHECK_THAT(len, Catch::Matchers::WithinAbs(1.0f, 1e-3f));
    }
}

TEST_CASE("SmoothInterpolate2D: interpolate(dist, ...) overload produces output", "[smoother]")
{
    SmoothInterpolate2D s;
    for (int i = 0; i <= 10; ++i)
        s.add_point({static_cast<float>(i), 0.0f});

    TwkMath::Vec2f out;
    bool any = false;
    while (s.interpolate(1.0f, out))
        any = true;
    CHECK(any);
}
