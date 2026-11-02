#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include <optional>
#include <string>
#include <jm/json.h>

struct Opt
{
    std::optional<int> n;
    std::optional<std::string> s;
};
JM_DEFINE_JSON_INLINE(Opt, n, s)

TEST_CASE("optional roundtrip: value / null")
{
    const Opt o{42, {}};
    const jm::json j = o;
    CHECK(j["n"] == 42);
    CHECK(j["s"].is_null());

    const Opt back = j.get<Opt>();
    CHECK(back.n.has_value());
    CHECK(*back.n == 42);
    CHECK_FALSE(back.s.has_value());
}

TEST_CASE("optional missing field keeps disengaged state")
{
    const jm::json j = jm::json::parse("{}");
    Opt o;
    j.get_to(o);
    CHECK_FALSE(o.n.has_value());
}
