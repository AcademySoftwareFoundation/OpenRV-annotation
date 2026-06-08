#include <TwkPaint/Paint.h>
#include <TwkPaint/Path.h>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

using namespace TwkPaint;

// Helper: build geometry and return true if output is non-empty and internally
// consistent (all triangle indices within vertex bounds, even vertex count).
static bool geometryIsValid(Path& p, Path::JoinStyle join, Path::CapStyle cap)
{
    p.computeGeometry(join, cap, Path::NoTexture, Path::QualityAlgorithm);
    const auto& verts = p.outputPoints();
    const auto& tris  = p.outputTriangles();
    if (verts.empty() || tris.empty()) return false;
    for (const auto& tri : tris)
    {
        if (tri.x >= verts.size()) return false;
        if (tri.y >= verts.size()) return false;
        if (tri.z >= verts.size()) return false;
    }
    return true;
}

// ── Path geometry ─────────────────────────────────────────────────────────────

TEST_CASE("Empty path produces no geometry", "[path]")
{
    Path p;
    p.setContantWidth(0.01f);
    p.computeGeometry(Path::RoundJoin, Path::RoundCap, Path::NoTexture, Path::QualityAlgorithm);
    CHECK(p.outputPoints().empty());
    CHECK(p.outputTriangles().empty());
}

// A single input point is treated as a stamp (a dot), not a line segment.
// filterPoints() produces n==1, so addStamp() is called and geometry IS emitted.
TEST_CASE("Single point produces a stamp (non-empty geometry)", "[path]")
{
    Path p;
    p.setContantWidth(0.01f);
    p.add(Point(0.5f, 0.5f));
    p.computeGeometry(Path::RoundJoin, Path::RoundCap, Path::NoTexture, Path::QualityAlgorithm);
    CHECK_FALSE(p.outputPoints().empty());
    CHECK_FALSE(p.outputTriangles().empty());
}

TEST_CASE("Two points produce valid geometry", "[path]")
{
    Path p;
    p.setContantWidth(0.02f);
    p.add(Point(0.0f, 0.0f));
    p.add(Point(1.0f, 0.0f));
    CHECK(geometryIsValid(p, Path::BevelJoin, Path::FlatCap));
}

TEST_CASE("Three collinear points produce valid geometry", "[path]")
{
    Path p;
    p.setContantWidth(0.02f);
    p.add(Point(0.0f, 0.0f));
    p.add(Point(0.5f, 0.0f));
    p.add(Point(1.0f, 0.0f));
    CHECK(geometryIsValid(p, Path::RoundJoin, Path::RoundCap));
}

TEST_CASE("Triangle path (sharp corner) produces valid geometry", "[path]")
{
    Path p;
    p.setContantWidth(0.02f);
    p.add(Point(0.0f, 0.0f));
    p.add(Point(0.5f, 1.0f));
    p.add(Point(1.0f, 0.0f));
    CHECK(geometryIsValid(p, Path::MiterJoin, Path::SquareCap));
}

TEST_CASE("All join styles produce valid geometry", "[path]")
{
    for (auto join : {Path::NoJoin, Path::BevelJoin, Path::MiterJoin, Path::RoundJoin})
    {
        Path p;
        p.setContantWidth(0.02f);
        p.add(Point(0.0f, 0.0f));
        p.add(Point(0.5f, 0.5f));
        p.add(Point(1.0f, 0.0f));
        CHECK(geometryIsValid(p, join, Path::FlatCap));
    }
}

TEST_CASE("All cap styles produce valid geometry", "[path]")
{
    for (auto cap : {Path::FlatCap, Path::SquareCap, Path::RoundCap})
    {
        Path p;
        p.setContantWidth(0.02f);
        p.add(Point(0.0f, 0.0f));
        p.add(Point(1.0f, 0.0f));
        CHECK(geometryIsValid(p, Path::RoundJoin, cap));
    }
}

TEST_CASE("Triangle indices are within vertex bounds", "[path]")
{
    Path p;
    p.setContantWidth(0.02f);
    p.add(Point(0.0f, 0.0f));
    p.add(Point(0.3f, 0.7f));
    p.add(Point(0.6f, 0.2f));
    p.add(Point(1.0f, 0.5f));
    p.computeGeometry(Path::RoundJoin, Path::RoundCap, Path::NoTexture, Path::QualityAlgorithm);

    const auto& verts = p.outputPoints();
    const auto& tris  = p.outputTriangles();
    REQUIRE_FALSE(verts.empty());
    REQUIRE_FALSE(tris.empty());
    for (const auto& tri : tris)
    {
        CHECK(tri.x < verts.size());
        CHECK(tri.y < verts.size());
        CHECK(tri.z < verts.size());
    }
}

TEST_CASE("Variable-width stroke produces valid geometry", "[path]")
{
    Path p;
    p.add(Point(0.0f, 0.0f), 0.01f);
    p.add(Point(0.5f, 0.5f), 0.05f);
    p.add(Point(1.0f, 0.0f), 0.01f);
    CHECK(geometryIsValid(p, Path::RoundJoin, Path::RoundCap));
}

TEST_CASE("clear() resets output arrays and allows re-use", "[path]")
{
    Path p;
    p.setContantWidth(0.02f);
    p.add(Point(0.0f, 0.0f));
    p.add(Point(1.0f, 0.0f));
    p.computeGeometry(Path::RoundJoin, Path::RoundCap, Path::NoTexture, Path::QualityAlgorithm);
    REQUIRE_FALSE(p.outputPoints().empty());

    // clear() immediately empties all output arrays without a computeGeometry call.
    p.clear();
    CHECK(p.outputPoints().empty());
    CHECK(p.outputTriangles().empty());

    // The path can be re-used after clear().
    p.setContantWidth(0.02f);
    p.add(Point(0.0f, 0.5f));
    p.add(Point(1.0f, 0.5f));
    p.computeGeometry(Path::RoundJoin, Path::RoundCap, Path::NoTexture, Path::QualityAlgorithm);
    CHECK_FALSE(p.outputPoints().empty());
}

TEST_CASE("TexCoords length matches vertex count", "[path]")
{
    Path p;
    p.setContantWidth(0.02f);
    p.add(Point(0.0f, 0.0f));
    p.add(Point(0.5f, 0.5f));
    p.add(Point(1.0f, 0.0f));
    p.computeGeometry(Path::RoundJoin, Path::RoundCap, Path::NoTexture, Path::QualityAlgorithm);
    CHECK(p.outputTexCoords().size() == p.outputPoints().size());
}

TEST_CASE("Many points produce consistent geometry", "[path]")
{
    Path p;
    p.setContantWidth(0.01f);
    const int N = 50;
    for (int i = 0; i < N; ++i)
    {
        float t = static_cast<float>(i) / (N - 1);
        p.add(Point(t, 0.5f * std::sin(t * 6.28f)));
    }
    CHECK(geometryIsValid(p, Path::RoundJoin, Path::RoundCap));

    // Directionalities should also be populated
    p.computeGeometry(Path::RoundJoin, Path::RoundCap, Path::VerticallySymmetric,
                      Path::QualityAlgorithm, true);
    CHECK_FALSE(p.outputDirectionalities().empty());
}

TEST_CASE("FastAlgorithm produces valid geometry", "[path]")
{
    Path p;
    p.setContantWidth(0.02f);
    p.add(Point(0.0f, 0.0f));
    p.add(Point(0.5f, 0.5f));
    p.add(Point(1.0f, 0.0f));
    p.computeGeometry(Path::RoundJoin, Path::RoundCap, Path::NoTexture, Path::FastAlgorithm);
    CHECK_FALSE(p.outputPoints().empty());
    CHECK_FALSE(p.outputTriangles().empty());
}

TEST_CASE("RadiallySymmetric texture style populates tex coords", "[path]")
{
    Path p;
    p.setContantWidth(0.02f);
    p.add(Point(0.0f, 0.0f));
    p.add(Point(0.5f, 0.0f));
    p.add(Point(1.0f, 0.0f));
    p.computeGeometry(Path::RoundJoin, Path::RoundCap, Path::RadiallySymmetric,
                      Path::QualityAlgorithm);
    REQUIRE_FALSE(p.outputPoints().empty());
    CHECK(p.outputTexCoords().size() == p.outputPoints().size());
}

TEST_CASE("Color add() overload produces valid geometry", "[path]")
{
    Path p;
    Color red(1.0f, 0.0f, 0.0f, 1.0f);
    p.add(Point(0.0f, 0.0f), red);
    p.add(Point(0.5f, 0.5f), red);
    p.add(Point(1.0f, 0.0f), red);
    p.computeGeometry(Path::RoundJoin, Path::RoundCap, Path::NoTexture, Path::QualityAlgorithm);
    CHECK_FALSE(p.outputPoints().empty());
    CHECK_FALSE(p.outputTriangles().empty());
}

TEST_CASE("Width-and-color add() overload produces valid geometry", "[path]")
{
    Path p;
    Color blue(0.0f, 0.0f, 1.0f, 1.0f);
    p.add(Point(0.0f, 0.0f), 0.01f, blue);
    p.add(Point(0.5f, 0.5f), 0.03f, blue);
    p.add(Point(1.0f, 0.0f), 0.01f, blue);
    p.computeGeometry(Path::RoundJoin, Path::RoundCap, Path::NoTexture, Path::QualityAlgorithm);
    CHECK_FALSE(p.outputPoints().empty());
    CHECK_FALSE(p.outputTriangles().empty());
}

TEST_CASE("splatOnly flag produces non-empty geometry", "[path]")
{
    Path p;
    p.setContantWidth(0.02f);
    p.add(Point(0.0f, 0.0f));
    p.add(Point(0.5f, 0.0f));
    p.add(Point(1.0f, 0.0f));
    p.computeGeometry(Path::RoundJoin, Path::RoundCap, Path::NoTexture, Path::QualityAlgorithm,
                      true, 1.0f, /*splatOnly=*/true);
    CHECK_FALSE(p.outputPoints().empty());
    CHECK_FALSE(p.outputTriangles().empty());
}
