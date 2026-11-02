// 本 target 由 CMake 统一定义 JSON_DIAGNOSTICS=1（见 CMakeLists）。
// 不要在源码中重复定义，也不要只对单个 TU 定义。
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include <jm/json.h>

struct Diag
{
    int a = 0;
    int b = 0;
};
JM_DEFINE_JSON_INLINE(Diag, a, b)

TEST_CASE("exception message carries JSON path under JSON_DIAGNOSTICS")
{
    const jm::json j = jm::json::parse(R"({"a": 1, "b": {"deep": true}})");
    Diag d;
    // 3.11.3 格式：消息前缀附加 JSON Pointer（如 "(/b) "）；
    // 断言不绑定具体措辞，只断言路径子串存在。
    // 路径来自 parser 的 set_parents() 建立的 m_parent 链与 get_arithmetic_value 传入的 &j。
    CHECK_THROWS_WITH_AS(j.get_to(d), doctest::Contains("/b"), jm::json::exception);
}
