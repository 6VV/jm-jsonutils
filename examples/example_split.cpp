// 声明/定义分离用法：类型头只放声明（不拉完整 nlohmann），定义集中一个 cpp。
#include <iostream>

#include <jm/json.h>

#include "user_types.h"

int main()
{
    const Config c{"127.0.0.1", 8080};

    const jm::json j = c;
    std::cout << j.dump() << std::endl;

    Config back;
    j.get_to(back);
    std::cout << "port=" << back.port << std::endl;
    return 0;
}
