#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include <nlohmann/json.hpp>

TEST_CASE("third-party toolchain smoke")
{
    const auto j = nlohmann::json::parse(R"({"a":1,"b":[1,2]})");
    CHECK(j["a"] == 1);
    CHECK(j["b"].size() == 2);
    CHECK(j.dump() == R"({"a":1,"b":[1,2]})");
}
