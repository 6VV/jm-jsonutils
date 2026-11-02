// jm/json_value.h —— 完整值类型层。
// 引入完整 nlohmann/json.hpp，并提供枚举 / std::optional 的补充特化。
// std::pair、容器、智能指针为 nlohmann 内置能力，此处不写。
#pragma once

#include <type_traits>

#include <nlohmann/json.hpp>

#ifdef JSON_HAS_CPP_17
#include <optional>
#endif

namespace jm
{
using json = nlohmann::json;
} // namespace jm

// ---- adl_serializer 补充特化（后续任务在此追加）----
