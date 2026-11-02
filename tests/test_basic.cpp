#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include <map>
#include <string>
#include <utility>
#include <vector>
#include <jm/json.h>

struct Point
{
    int x = 0;
    int y = 0;
};
JM_DEFINE_JSON_INLINE(Point, x, y)

struct User
{
    std::string name;
    int age = 0;
    std::vector<Point> points;
};
JM_DEFINE_JSON_INLINE(User, name, age, points)

enum class Status
{
    Idle = 0,
    Running = 1
};

struct Task
{
    std::string name;
    Status status = Status::Idle;
};
JM_DEFINE_JSON_INLINE(Task, name, status)

TEST_CASE("scalar struct roundtrip")
{
    const Point p{1, 2};
    const jm::json j = p;
    CHECK(j == jm::json{{"x", 1}, {"y", 2}});

    const Point q = j.get<Point>();
    CHECK(q.x == 1);
    CHECK(q.y == 2);
}

TEST_CASE("nested struct with container field")
{
    const User u{"tom", 18, {{1, 2}, {3, 4}}};
    const jm::json j = u;
    CHECK(j["points"].size() == 2);

    const User v = j.get<User>();
    CHECK(v.name == "tom");
    CHECK(v.age == 18);
    CHECK(v.points.size() == 2);
    CHECK(v.points[1].x == 3);
}

TEST_CASE("enum field in struct via adl_serializer")
{
    const Task t{"deploy", Status::Running};
    const jm::json j = t;
    CHECK(j["status"] == 1);

    const Task back = j.get<Task>();
    CHECK(back.status == Status::Running);
}

TEST_CASE("missing field keeps current value")
{
    const jm::json j = jm::json::parse(R"({"age": 20})");
    User u;
    u.name = "preset";
    j.get_to(u);
    CHECK(u.name == "preset"); // 缺 name 字段 → 保留当前值
    CHECK(u.age == 20);
    CHECK(u.points.empty());
}

TEST_CASE("containers of struct via nlohmann builtin")
{
    const std::map<std::string, Point> m{{"a", {1, 1}}};
    const jm::json j = m;
    const auto back = j.get<std::map<std::string, Point>>();
    CHECK(back.at("a").y == 1);
}

// std::pair 为 nlohmann 内置（本库不特化，此处验证内置语义供文档锚定）
TEST_CASE("std::pair builtin roundtrip as 2-element array")
{
    const std::pair<int, std::string> p{7, "seven"};
    const jm::json j = p;
    CHECK(j.is_array());
    CHECK(j.size() == 2);
    CHECK(j[0] == 7);
    CHECK(j[1] == "seven");

    const auto back = j.get<std::pair<int, std::string>>();
    CHECK(back.first == 7);
    CHECK(back.second == "seven");
}

TEST_CASE("std::pair builtin error semantics")
{
    std::pair<int, int> p;
    // 内置 from_json 用 at(0)/at(1)：越界抛 out_of_range，非 array 抛 type_error
    CHECK_THROWS_AS(jm::json::parse(R"([1])").get_to(p), jm::json::out_of_range);
    CHECK_THROWS_AS(jm::json::parse("{}").get_to(p), jm::json::type_error);
    // 多余元素被内置实现忽略（[1,2,3] → (1,2)），这是 nlohmann 事实行为
    jm::json::parse(R"([1,2,3])").get_to(p);
    CHECK(p.first == 1);
    CHECK(p.second == 2);
}
