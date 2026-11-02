// header-only 用法：类型 + 宏都放头文件/本文件。
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include <jm/json.h>

enum class Status
{
    Idle = 0,
    Running = 1
};

struct Point
{
    int x = 0;
    int y = 0;
};
JM_DEFINE_JSON_INLINE(Point, x, y)

struct Task
{
    std::string name;
    Status status = Status::Idle;
    std::vector<Point> path;
};
JM_DEFINE_JSON_INLINE(Task, name, status, path)

int main()
{
    const Task t{"deploy", Status::Running, {{1, 2}, {3, 4}}};

    const jm::json j = t;
    std::cout << j.dump(2) << std::endl;

    const Task back = j.get<Task>();
    std::cout << "name=" << back.name << " points=" << back.path.size()
              << " status=" << static_cast<int>(back.status) << std::endl;
    return 0;
}
