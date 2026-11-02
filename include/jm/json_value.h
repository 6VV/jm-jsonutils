// jm/json_value.h —— 完整值类型层。
// 引入完整 nlohmann/json.hpp，并提供 std::optional 的补充特化。
// std::pair、容器等为 nlohmann 内置能力，此处不写。
#pragma once

#include <type_traits>

#include <nlohmann/json.hpp>

// C++17 语言模式检测：nlohmann/json.hpp 在末尾会 #undef JSON_HAS_CPP_*，
// 因此此处按标准宏自行判断（与 nlohmann 内部检测逻辑一致）。
#if (defined(__cplusplus) && __cplusplus >= 201703L) || \
    (defined(_MSVC_LANG) && _MSVC_LANG >= 201703L) || \
    (defined(_HAS_CXX17) && _HAS_CXX17)
#define JM_DETAIL_HAS_CPP17 1
#include <optional>
#endif

namespace jm
{
using json = nlohmann::json;
} // namespace jm

// ---- adl_serializer 补充特化 ----
namespace nlohmann
{
#ifdef JM_DETAIL_HAS_CPP17
// std::optional（C++17）：空值 ↔ null。
template <typename T>
struct adl_serializer<std::optional<T>>
{
    static void to_json(json &j, const std::optional<T> &o)
    {
        if (o.has_value())
        {
            j = *o;
        }
        else
        {
            j = nullptr;
        }
    }

    static void from_json(const json &j, std::optional<T> &o)
    {
        if (j.is_null())
        {
            o.reset();
        }
        else
        {
            o = j.get<T>();
        }
    }
};
#endif
} // namespace nlohmann

// 内部检测宏仅服务于本头文件，不泄漏到消费者命名空间。
#undef JM_DETAIL_HAS_CPP17
