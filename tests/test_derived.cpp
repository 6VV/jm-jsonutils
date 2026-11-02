#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include <string>
#include <jm/json.h>

struct Base
{
    int id = 0;
};
JM_DEFINE_JSON_INLINE(Base, id)

struct Derived : Base
{
    std::string tag;
};
JM_DEFINE_JSON_DERIVED_INLINE(Base, Derived, tag)

TEST_CASE("derived serialization includes parent fields")
{
    Derived d;
    d.id = 9;
    d.tag = "x";

    const jm::json j = d;
    CHECK(j["id"] == 9);
    CHECK(j["tag"] == "x");
}

TEST_CASE("derived deserialization parses both levels")
{
    const jm::json j = jm::json::parse(R"({"id": 3, "tag": "t"})");
    Derived d;
    j.get_to(d);
    CHECK(d.id == 3);
    CHECK(d.tag == "t");
}

TEST_CASE("derived missing parent field keeps default")
{
    const jm::json j = jm::json::parse(R"({"tag": "t"})");
    Derived d;
    j.get_to(d);
    CHECK(d.id == 0);
    CHECK(d.tag == "t");
}

TEST_CASE("derived non-object root throws")
{
    const jm::json j = jm::json::parse("[1]");
    Derived d;
    CHECK_THROWS_AS(j.get_to(d), jm::json::type_error);
}

// 非 inline DERIVED：定义与使用同 TU，验证非 inline 宏体。
struct PlainBase
{
    int id = 0;
};
JM_DEFINE_JSON_INLINE(PlainBase, id)

struct PlainDerived : PlainBase
{
    std::string label;
};
JM_DEFINE_JSON_DERIVED(PlainBase, PlainDerived, label)

TEST_CASE("non-inline derived macro roundtrip")
{
    PlainDerived d;
    d.id = 4;
    d.label = "z";
    const jm::json j = d;
    CHECK(j["id"] == 4);
    CHECK(j["label"] == "z");

    PlainDerived back;
    j.get_to(back);
    CHECK(back.id == 4);
    CHECK(back.label == "z");
}
