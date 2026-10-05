#include <TwkPaint/StampSeed.h>
#include <catch2/catch_test_macros.hpp>
#include <cstring>

using namespace TwkPaint;

TEST_CASE("seed_from_annotation_id returns 0 for empty input", "[stampseed]")
{
    CHECK(seed_from_annotation_id("", 0) == 0);
    CHECK(seed_from_annotation_id(nullptr, 0) == 0);
}

TEST_CASE("seed_from_annotation_id is stable for a known UUID", "[stampseed]")
{
    const char* id = "83c35327-acf3-489e-b456-d13064d84aa5";
    CHECK(seed_from_annotation_id(id, std::strlen(id)) == 3213262474u);
}

TEST_CASE("seed_from_annotation_id differs for different ids", "[stampseed]")
{
    const char* a = "83c35327-acf3-489e-b456-d13064d84aa5";
    const char* b = "4dd676c6-9ffa-4fd4-930d-5f37c8a0bb8d";
    CHECK(seed_from_annotation_id(a, std::strlen(a)) != seed_from_annotation_id(b, std::strlen(b)));
}
