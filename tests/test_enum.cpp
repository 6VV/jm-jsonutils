#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include <cstdint>
#include <jm/json.h>

enum Color
{
    Red,
    Green = 5
};

enum class Mode : std::uint8_t
{
    Off,
    On
};

TEST_CASE("enum serializes as underlying integer")
{
    const jm::json j = Green;
    CHECK(j == 5);
    const jm::json j2 = Mode::On;
    CHECK(j2 == 1);

    const Color back = j.get<Color>();
    CHECK(back == Green);
    const Mode m2 = j2.get<Mode>();
    CHECK(m2 == Mode::On);
}

TEST_CASE("enum from non-integer throws type_error")
{
    const jm::json j = jm::json::parse(R"("oops")");
    CHECK_THROWS_AS(j.get<Color>(), jm::json::type_error);
}
