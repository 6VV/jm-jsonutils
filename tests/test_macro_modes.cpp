#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include <string>
#include <jm/json.h>

// 宏在用户 namespace 内展开（ADL 可达，无需 bridge）
namespace game
{
struct Vec2
{
    float x = 0.f;
    float y = 0.f;
};
JM_DEFINE_JSON_INLINE(Vec2, x, y)
} // namespace game

// 单向：只序列化（如日志模型）
struct LogLine
{
    long ts = 0;
};
JM_DEFINE_JSON_TO_INLINE(LogLine, ts)

// 单向：只反序列化（如只读配置）
struct Cfg
{
    int port = 0;
};
JM_DEFINE_JSON_FROM_INLINE(Cfg, port)

// 单个字段与较多字段
struct One
{
    int only = 0;
};
JM_DEFINE_JSON_INLINE(One, only)

struct Many
{
    int f1 = 0, f2 = 0, f3 = 0, f4 = 0, f5 = 0, f6 = 0, f7 = 0, f8 = 0;
};
JM_DEFINE_JSON_INLINE(Many, f1, f2, f3, f4, f5, f6, f7, f8)

// 63 字段上限验证
struct Wide
{
    int f1 = 1, f2 = 2, f3 = 3, f4 = 4, f5 = 5, f6 = 6, f7 = 7, f8 = 8;
    int f9 = 9, f10 = 10, f11 = 11, f12 = 12, f13 = 13, f14 = 14, f15 = 15, f16 = 16;
    int f17 = 17, f18 = 18, f19 = 19, f20 = 20, f21 = 21, f22 = 22, f23 = 23, f24 = 24;
    int f25 = 25, f26 = 26, f27 = 27, f28 = 28, f29 = 29, f30 = 30, f31 = 31, f32 = 32;
    int f33 = 33, f34 = 34, f35 = 35, f36 = 36, f37 = 37, f38 = 38, f39 = 39, f40 = 40;
    int f41 = 41, f42 = 42, f43 = 43, f44 = 44, f45 = 45, f46 = 46, f47 = 47, f48 = 48;
    int f49 = 49, f50 = 50, f51 = 51, f52 = 52, f53 = 53, f54 = 54, f55 = 55, f56 = 56;
    int f57 = 57, f58 = 58, f59 = 59, f60 = 60, f61 = 61, f62 = 62, f63 = 63;
};
JM_DEFINE_JSON_INLINE(Wide, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14,
    f15, f16, f17, f18, f19, f20, f21, f22, f23, f24, f25, f26, f27, f28, f29, f30, f31,
    f32, f33, f34, f35, f36, f37, f38, f39, f40, f41, f42, f43, f44, f45, f46, f47, f48,
    f49, f50, f51, f52, f53, f54, f55, f56, f57, f58, f59, f60, f61, f62, f63)

TEST_CASE("macro expands inside user namespace")
{
    const game::Vec2 v{1.5f, 2.5f};
    const jm::json j = v;
    CHECK(j["x"] == 1.5f);
    const auto back = j.get<game::Vec2>();
    CHECK(back.y == 2.5f);
}

TEST_CASE("to-only macro")
{
    const LogLine l{123};
    const jm::json j = l;
    CHECK(j["ts"] == 123);
}

TEST_CASE("from-only macro")
{
    const jm::json j = jm::json::parse(R"({"port": 8080})");
    Cfg c;
    j.get_to(c);
    CHECK(c.port == 8080);
}

TEST_CASE("single field and 8-field lists")
{
    const One o{9};
    const jm::json j1 = o;
    CHECK(j1["only"] == 9);

    const Many m;
    const jm::json j2 = m;
    CHECK(j2.size() == 8);
    const auto back = j2.get<Many>();
    CHECK(back.f8 == 0);
}

TEST_CASE("63 fields at documented upper bound")
{
    const Wide w;
    const jm::json j = w;
    CHECK(j.size() == 63);
    CHECK(j["f1"] == 1);
    CHECK(j["f63"] == 63);
    const auto back = j.get<Wide>();
    CHECK(back.f32 == 32);
    CHECK(back.f63 == 63);
}
