#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include <jm/json.h>

#include "support/split_types.h"

TEST_CASE("declare/define split works across translation units")
{
    const SplitUser u{"ann", 99};
    const jm::json j = u; // 链接到 split_defs.cpp 的定义
    CHECK(j["score"] == 99);

    SplitUser v;
    j.get_to(v);
    CHECK(v.name == "ann");
}

// 非 inline 单向宏：在 cpp 中定义，验证宏体形状（与分离模式无关）。
struct SplitToOnly
{
    int code = 0;
};
JM_DEFINE_JSON_TO(SplitToOnly, code)

struct SplitFromOnly
{
    int level = 0;
};
JM_DEFINE_JSON_FROM(SplitFromOnly, level)

TEST_CASE("non-inline to-only macro")
{
    const SplitToOnly v{7};
    const jm::json j = v;
    CHECK(j["code"] == 7);
}

TEST_CASE("non-inline from-only macro")
{
    const jm::json j = jm::json::parse(R"({"level": 3})");
    SplitFromOnly v;
    j.get_to(v);
    CHECK(v.level == 3);
}
