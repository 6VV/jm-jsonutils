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
