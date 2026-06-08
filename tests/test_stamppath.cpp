#include <TwkPaint/StampPath.h>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

using namespace TwkPaint;

// ── Helpers ───────────────────────────────────────────────────────────────────

static std::vector<StampInstance> drainStamps(StampPath& sp)
{
    std::vector<StampInstance> out;
    StampInstance s;
    while (sp.next(s))
        out.push_back(s);
    return out;
}

// ── Basic placement ───────────────────────────────────────────────────────────

TEST_CASE("No stamps before any points are added", "[stamppath]")
{
    StampPath sp;
    StampInstance s;
    CHECK_FALSE(sp.next(s));
}

TEST_CASE("No stamps after only one point", "[stamppath]")
{
    StampPath sp;
    sp.add_point({0.0f, 0.0f});
    CHECK(drainStamps(sp).empty());
}

TEST_CASE("Stamps appear after two points on a long segment", "[stamppath]")
{
    BrushParams bp;
    bp.radius  = 0.05f;
    bp.spacing = 0.1f;

    StampPath sp(bp);
    sp.add_point({0.0f, 0.0f});
    sp.add_point({1.0f, 0.0f});

    CHECK_FALSE(drainStamps(sp).empty());
}

TEST_CASE("Stamp positions lie on the path (horizontal segment)", "[stamppath]")
{
    BrushParams bp;
    bp.radius  = 0.02f;
    bp.spacing = 0.05f;

    StampPath sp(bp);
    sp.add_point({0.0f, 0.0f});
    sp.add_point({1.0f, 0.0f});

    for (const auto& s : drainStamps(sp))
    {
        CHECK(s.pos.x >= -0.01f);
        CHECK(s.pos.x <= 1.01f);
        CHECK_THAT(s.pos.y, Catch::Matchers::WithinAbs(0.0f, 1e-4f));
    }
}

TEST_CASE("Stamp radius matches BrushParams default", "[stamppath]")
{
    BrushParams bp;
    bp.radius  = 0.07f;
    bp.spacing = 0.1f;

    StampPath sp(bp);
    sp.add_point({0.0f, 0.0f});
    sp.add_point({1.0f, 0.0f});

    for (const auto& s : drainStamps(sp))
        CHECK_THAT(s.radius, Catch::Matchers::WithinAbs(0.07f, 1e-5f));
}

TEST_CASE("Stamp opacity matches BrushParams default", "[stamppath]")
{
    BrushParams bp;
    bp.radius  = 0.05f;
    bp.opacity = 0.6f;
    bp.spacing = 0.1f;

    StampPath sp(bp);
    sp.add_point({0.0f, 0.0f});
    sp.add_point({1.0f, 0.0f});

    for (const auto& s : drainStamps(sp))
        CHECK_THAT(s.opacity, Catch::Matchers::WithinAbs(0.6f, 1e-5f));
}

// ── Per-point overrides ───────────────────────────────────────────────────────

TEST_CASE("Per-point radius override is used", "[stamppath]")
{
    BrushParams bp;
    bp.radius  = 0.02f;
    bp.spacing = 0.05f;

    StampPath sp(bp);
    sp.add_point({0.0f, 0.0f}, /*radius=*/0.10f);
    sp.add_point({1.0f, 0.0f}, /*radius=*/0.10f);

    auto stamps = drainStamps(sp);
    REQUIRE_FALSE(stamps.empty());
    // All stamps should interpolate toward the override value, not the default.
    for (const auto& s : stamps)
        CHECK(s.radius > 0.02f - 1e-4f);
}

TEST_CASE("Negative per-point override falls back to BrushParams", "[stamppath]")
{
    BrushParams bp;
    bp.radius  = 0.05f;
    bp.spacing = 0.1f;

    StampPath sp(bp);
    sp.add_point({0.0f, 0.0f}, -1.f, -1.f, -1.f, -1.f);
    sp.add_point({1.0f, 0.0f}, -1.f, -1.f, -1.f, -1.f);

    for (const auto& s : drainStamps(sp))
        CHECK_THAT(s.radius, Catch::Matchers::WithinAbs(0.05f, 1e-5f));
}

// ── Spacing ───────────────────────────────────────────────────────────────────

TEST_CASE("Smaller spacing produces more stamps", "[stamppath]")
{
    BrushParams bpClose, bpFar;
    bpClose.radius = bpFar.radius = 0.02f;
    bpClose.spacing               = 0.05f;
    bpFar.spacing                 = 0.20f;

    StampPath spClose(bpClose), spFar(bpFar);
    spClose.add_point({0.0f, 0.0f});
    spClose.add_point({1.0f, 0.0f});
    spFar.add_point({0.0f, 0.0f});
    spFar.add_point({1.0f, 0.0f});

    CHECK(drainStamps(spClose).size() > drainStamps(spFar).size());
}

// ── Reset ─────────────────────────────────────────────────────────────────────

TEST_CASE("reset() clears state so next point behaves like cold start", "[stamppath]")
{
    BrushParams bp;
    bp.radius  = 0.05f;
    bp.spacing = 0.1f;

    StampPath sp(bp);
    sp.add_point({0.0f, 0.0f});
    sp.add_point({1.0f, 0.0f});
    drainStamps(sp); // consume

    sp.reset();
    sp.add_point({0.0f, 0.0f});
    // Single point after reset should produce no stamps.
    CHECK(drainStamps(sp).empty());
}

TEST_CASE("reset() with new params uses updated params", "[stamppath]")
{
    BrushParams bp;
    bp.radius  = 0.02f;
    bp.spacing = 0.1f;

    StampPath sp(bp);
    sp.add_point({0.0f, 0.0f});
    sp.add_point({1.0f, 0.0f});
    drainStamps(sp);

    BrushParams bp2;
    bp2.radius  = 0.08f;
    bp2.spacing = 0.1f;
    sp.reset(bp2);
    sp.add_point({0.0f, 0.0f});
    sp.add_point({1.0f, 0.0f});

    for (const auto& s : drainStamps(sp))
        CHECK_THAT(s.radius, Catch::Matchers::WithinAbs(0.08f, 1e-5f));
}

// ── Multi-segment path ────────────────────────────────────────────────────────

TEST_CASE("Multi-segment path produces stamps along each segment", "[stamppath]")
{
    BrushParams bp;
    bp.radius  = 0.02f;
    bp.spacing = 0.05f;

    StampPath sp(bp);
    sp.add_point({0.0f, 0.0f});
    sp.add_point({0.5f, 0.0f});
    sp.add_point({0.5f, 0.5f});
    sp.add_point({1.0f, 0.5f});

    StampInstance s;
    int total = 0;
    while (sp.next(s))
        ++total;
    CHECK(total > 3);
}

// ── Per-point angle / squish overrides ───────────────────────────────────────

TEST_CASE("Per-point angle override is reflected in stamp", "[stamppath]")
{
    BrushParams bp;
    bp.radius  = 0.05f;
    bp.spacing = 0.1f;
    bp.angle   = 0.0f;

    StampPath sp(bp);
    sp.add_point({0.0f, 0.0f}, -1.f, -1.f, /*angle=*/45.0f, -1.f);
    sp.add_point({1.0f, 0.0f}, -1.f, -1.f, /*angle=*/45.0f, -1.f);

    auto stamps = drainStamps(sp);
    REQUIRE_FALSE(stamps.empty());
    for (const auto& s : stamps)
        CHECK(s.angle > 0.0f - 1e-4f); // angle biased toward 45°
}

TEST_CASE("Per-point squish override is reflected in stamp", "[stamppath]")
{
    BrushParams bp;
    bp.radius  = 0.05f;
    bp.spacing = 0.1f;
    bp.squish  = 1.0f;

    StampPath sp(bp);
    sp.add_point({0.0f, 0.0f}, -1.f, -1.f, -1.f, /*squish=*/0.5f);
    sp.add_point({1.0f, 0.0f}, -1.f, -1.f, -1.f, /*squish=*/0.5f);

    auto stamps = drainStamps(sp);
    REQUIRE_FALSE(stamps.empty());
    for (const auto& s : stamps)
        CHECK(s.squish < 1.0f + 1e-4f); // squish biased toward 0.5
}

// ── Proportional default spacing ─────────────────────────────────────────────

TEST_CASE("spacing=0 uses proportional default and still produces stamps", "[stamppath]")
{
    BrushParams bp;
    bp.radius  = 0.05f;
    bp.spacing = 0.0f; // trigger proportional default

    StampPath sp(bp);
    sp.add_point({0.0f, 0.0f});
    sp.add_point({1.0f, 0.0f});

    CHECK_FALSE(drainStamps(sp).empty());
}

// ── rotateToStroke ────────────────────────────────────────────────────────────

TEST_CASE("rotateToStroke=true produces stamps without crash", "[stamppath]")
{
    BrushParams bp;
    bp.radius         = 0.05f;
    bp.spacing        = 0.1f;
    bp.rotateToStroke = true;

    StampPath sp(bp);
    sp.add_point({0.0f, 0.0f});
    sp.add_point({1.0f, 0.0f});

    auto stamps = drainStamps(sp);
    CHECK_FALSE(stamps.empty());
}

// ── Jitter params (smoke tests) ───────────────────────────────────────────────

TEST_CASE("Jitter params do not crash and produce stamps", "[stamppath]")
{
    BrushParams bp;
    bp.radius         = 0.05f;
    bp.spacing        = 0.1f;
    bp.spacingJitter  = 0.5f;
    bp.opacityJitter  = 0.5f;
    bp.radiusJitter   = 0.5f;
    bp.rotationJitter = 45.0f;

    StampPath sp(bp);
    sp.add_point({0.0f, 0.0f});
    sp.add_point({1.0f, 0.0f});

    auto stamps = drainStamps(sp);
    CHECK_FALSE(stamps.empty());
    // Even with jitter, radius/opacity must stay in a reasonable range.
    for (const auto& s : stamps)
    {
        CHECK(s.radius > 0.0f);
        CHECK(s.opacity >= 0.0f);
        CHECK(s.opacity <= 1.0f);
    }
}
