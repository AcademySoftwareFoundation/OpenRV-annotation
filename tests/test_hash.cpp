#include <TwkMath/Hash.h>
#include <catch2/catch_test_macros.hpp>
#include <cstring>

using namespace TwkMath;

TEST_CASE("hash_u32 returns 0 for empty input", "[hash]")
{
    CHECK(hash_u32("", 0) == 0);
    CHECK(hash_u32(nullptr, 0) == 0);
}

TEST_CASE("hash_u32 is stable for a known string", "[hash]")
{
    const char* id = "83c35327-acf3-489e-b456-d13064d84aa5";
    CHECK(hash_u32(id, std::strlen(id)) == 3213262474u);
}

TEST_CASE("hash_u32 differs for different strings", "[hash]")
{
    const char* a = "83c35327-acf3-489e-b456-d13064d84aa5";
    const char* b = "4dd676c6-9ffa-4fd4-930d-5f37c8a0bb8d";
    CHECK(hash_u32(a, std::strlen(a)) != hash_u32(b, std::strlen(b)));
}

TEST_CASE("hash_u32 string_view overload matches pointer+length overload", "[hash]")
{
    const char* id = "83c35327-acf3-489e-b456-d13064d84aa5";
    CHECK(hash_u32(std::string_view(id)) == hash_u32(id, std::strlen(id)));
}
