// jm/json_macro.h —— 宏层：TO / FROM 核心，CODEC 组合糖。
//
// 生成的函数为 nlohmann ADL 标准签名：
//   void to_json(::jm::json &, const Type &);
//   void from_json(const ::jm::json &, Type &);
//
// 硬约束：宏必须在类型所在的 namespace 内展开（ADL 依赖），
//         否则 nlohmann 的容器/智能指针机制找不到这些函数。
//
// 反序列化语义：
//   缺字段        → 跳过，保留目标当前值
//   字段类型不匹配 → 抛 jm::json::type_error（nlohmann get 语义）
//   根不是 object  → 抛 jm::json::type_error（异常携带 &j 上下文，
//                    JSON_DIAGNOSTICS=1 时消息含 JSON 路径）
//
// 字段展开复用 nlohmann v3.11.3 自带的 NLOHMANN_JSON_PASTE（1..63 字段，
// MSVC 传统预处理器兼容由官方保证）；vendored 版本已锁 3.11.3。
#pragma once

#include <string>
#include <type_traits>

#include <jm/json_value.h>

// ---- 内部：单字段处理 ----
#define JM_DETAIL_TO_FIELD(x) j[#x] = t.x;

#define JM_DETAIL_FROM_FIELD(x) \
    { \
        auto it_ = j.find(#x); \
        if (it_ != j.end()) \
        { \
            it_->get_to(t.x); \
        } \
    }

// ---- 内部：函数体片段 ----
// type_error::create 为三参形式（id, what, context）；&j 使 JSON_DIAGNOSTICS
// 开启时异常携带路径。
#define JM_DETAIL_THROW_NOT_OBJECT() \
    throw ::jm::json::type_error::create( \
        302, "type must be object, but is " + std::string(j.type_name()), &j)

#define JM_DETAIL_TO_BODY(...) \
    { \
        NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(JM_DETAIL_TO_FIELD, __VA_ARGS__)) \
    }

#define JM_DETAIL_FROM_BODY(...) \
    { \
        if (!j.is_object()) \
        { \
            JM_DETAIL_THROW_NOT_OBJECT(); \
        } \
        NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(JM_DETAIL_FROM_FIELD, __VA_ARGS__)) \
    }

// ---- 公开：cpp 中的定义（非 inline，全程序仅一个 TU 展开）----
#define JM_DEFINE_JSON_TO(Type, ...) \
    void to_json(::jm::json &j, const Type &t) \
    JM_DETAIL_TO_BODY(__VA_ARGS__)

#define JM_DEFINE_JSON_FROM(Type, ...) \
    void from_json(const ::jm::json &j, Type &t) \
    JM_DETAIL_FROM_BODY(__VA_ARGS__)

#define JM_DEFINE_JSON(Type, ...) \
    JM_DEFINE_JSON_TO(Type, __VA_ARGS__) \
    JM_DEFINE_JSON_FROM(Type, __VA_ARGS__)

// ---- 公开：头文件中的定义（inline）----
#define JM_DEFINE_JSON_TO_INLINE(Type, ...) \
    inline void to_json(::jm::json &j, const Type &t) \
    JM_DETAIL_TO_BODY(__VA_ARGS__)

#define JM_DEFINE_JSON_FROM_INLINE(Type, ...) \
    inline void from_json(const ::jm::json &j, Type &t) \
    JM_DETAIL_FROM_BODY(__VA_ARGS__)

#define JM_DEFINE_JSON_INLINE(Type, ...) \
    JM_DEFINE_JSON_TO_INLINE(Type, __VA_ARGS__) \
    JM_DEFINE_JSON_FROM_INLINE(Type, __VA_ARGS__)

// ---- 公开：继承（先复用父类转换，再处理子类字段）----
// 注意参数顺序为 (Parent, Type, fields...)，static_assert 防写反。
#define JM_DEFINE_JSON_DERIVED(Parent, Type, ...) \
    void to_json(::jm::json &j, const Type &t) \
    { \
        static_assert(std::is_base_of<Parent, Type>::value, \
            "JM_DEFINE_JSON_DERIVED: first argument must be the base class"); \
        to_json(j, static_cast<const Parent &>(t)); \
        NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(JM_DETAIL_TO_FIELD, __VA_ARGS__)) \
    } \
    void from_json(const ::jm::json &j, Type &t) \
    { \
        static_assert(std::is_base_of<Parent, Type>::value, \
            "JM_DEFINE_JSON_DERIVED: first argument must be the base class"); \
        if (!j.is_object()) \
        { \
            JM_DETAIL_THROW_NOT_OBJECT(); \
        } \
        from_json(j, static_cast<Parent &>(t)); \
        NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(JM_DETAIL_FROM_FIELD, __VA_ARGS__)) \
    }

#define JM_DEFINE_JSON_DERIVED_INLINE(Parent, Type, ...) \
    inline void to_json(::jm::json &j, const Type &t) \
    { \
        static_assert(std::is_base_of<Parent, Type>::value, \
            "JM_DEFINE_JSON_DERIVED: first argument must be the base class"); \
        to_json(j, static_cast<const Parent &>(t)); \
        NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(JM_DETAIL_TO_FIELD, __VA_ARGS__)) \
    } \
    inline void from_json(const ::jm::json &j, Type &t) \
    { \
        static_assert(std::is_base_of<Parent, Type>::value, \
            "JM_DEFINE_JSON_DERIVED: first argument must be the base class"); \
        if (!j.is_object()) \
        { \
            JM_DETAIL_THROW_NOT_OBJECT(); \
        } \
        from_json(j, static_cast<Parent &>(t)); \
        NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(JM_DETAIL_FROM_FIELD, __VA_ARGS__)) \
    }
