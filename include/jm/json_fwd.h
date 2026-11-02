// jm/json_fwd.h —— 轻量声明层。
// 只引入 nlohmann 前向声明头，供"类型头声明 + 单个 cpp 定义"场景压制编译依赖。
// 注意：调用方 TU 使用 JSON 值仍需完整类型（include <jm/json.h>）。
#pragma once

#include <nlohmann/json_fwd.hpp>

namespace jm
{
using json = nlohmann::json;
} // namespace jm

// 分离模式声明宏（仅支持双向；单向 codec 请用 _INLINE 宏或手写声明）。
// 生成的签名与 JM_DEFINE_JSON* 的定义严格一致；仅依赖前向声明的 json 类型。
#define JM_DECLARE_JSON(Type) \
    void to_json(::jm::json &j, const Type &value); \
    void from_json(const ::jm::json &j, Type &value);
