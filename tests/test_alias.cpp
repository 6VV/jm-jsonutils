#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include <cstdint>
#include <jm/json_fwd.h>
#include <jm/json.h>

TEST_CASE("jm::json alias works")
{
    const jm::json j = jm::json::parse(R"({"a":1})");
    CHECK(j["a"] == 1);
    CHECK(j.dump() == R"({"a":1})");
}

TEST_CASE("int64 precision preserved")
{
    const int64_t big = 9007199254740993LL; // 2^53 + 1，double 无法精确表示
    const jm::json j = big;
    CHECK(j.get<int64_t>() == big);
}
