#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include <string>
#include <vector>
#include <jm/json.h>

struct Err
{
    int a = 0;
    int b = 0;
};
JM_DEFINE_JSON_INLINE(Err, a, b)

TEST_CASE("field type mismatch throws type_error")
{
    const jm::json j = jm::json::parse(R"({"a": "not a number"})");
    Err e;
    CHECK_THROWS_AS(j.get_to(e), jm::json::type_error);
}

TEST_CASE("non-object root throws type_error")
{
    Err e;
    CHECK_THROWS_AS(jm::json::parse("[1,2]").get_to(e), jm::json::type_error);
    CHECK_THROWS_AS(jm::json::parse("\"s\"").get_to(e), jm::json::type_error);
    CHECK_THROWS_AS(jm::json::parse("null").get_to(e), jm::json::type_error);
}

TEST_CASE("exception message format follows nlohmann")
{
    const jm::json j = jm::json::parse(R"({"a": "s"})");
    Err e;
    CHECK_THROWS_WITH(j.get_to(e), doctest::Contains("type must be number"));
}

TEST_CASE("vector element failure aborts container parse")
{
    const jm::json j = jm::json::parse(R"([1, 2, 3])");
    CHECK_THROWS_AS((j.get<std::vector<Err>>()), jm::json::type_error);
}
