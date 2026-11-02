# jm-jsonutils

C++14 纯头文件库：通过宏为结构体生成 JSON 序列化（nlohmann ADL `to_json` / `from_json`），基于 vendored [nlohmann/json](https://github.com/nlohmann/json) v3.11.3（版本锁死）。

- 命名空间 `jm`，值类型别名 `jm::json = nlohmann::json`
- 9 个公开宏，无策略维度组合
- 容器 / 嵌套结构体 / `std::pair` 由 nlohmann 自动传递，零额外代码
- 大整数 int64 精确（无 double 精度损失）
- 跨平台：Windows（MSVC 2019+）与 Linux（GCC 9+ / Clang 10+）
- 完全离线：`third_party/` 随仓库入库，构建不访问网络，无 submodule / FetchContent

## 平台与工具链

| 平台 | 编译器 | 生成器 | 要求 |
|---|---|---|---|
| Windows | MSVC 2019+ | Visual Studio 生成器 | CMake 3.20+；`/permissive-`、`/utf-8` 由库 target 自动附加 |
| Linux | GCC 9+ / Clang 10+ | Ninja 或 Make | CMake 3.20+ |

C++14 是底线；`std::optional` 在 C++17 及以上编译模式自动启用。

## 30 秒上手

```cpp
#include <jm/json.h>

struct User
{
    std::string name;
    int age = 0;
};
JM_DEFINE_JSON_INLINE(User, name, age)

User u{"tom", 18};
jm::json j = u;           // 序列化
User v = j.get<User>();   // 反序列化

// 容器自动传递：
std::map<std::string, std::vector<User>> db;
jm::json jd = db;                                  // 直接可用
auto back = jd.get<std::map<std::string, std::vector<User>>>();
```

## 使用约束（重要）

- **宏必须在类型所在的 namespace 内展开**（ADL 依赖）。跨 namespace 展开会导致
  nlohmann 的容器机制找不到生成的函数。
- key = 字段名字符串化（`#field`），不支持自定义 key —— 需要时手写 `to_json`
- 单次 1..63 个字段；空 struct 不应使用宏
- 分离模式（声明 + cpp 定义）仅支持双向 codec；单向 codec 请用 `_INLINE` 宏或手写声明
- `JM_DECLARE_JSON` 在类型头中展开时只需 `#include <jm/json_fwd.h>`（轻量）；引 `<jm/json.h>` 同样可用

## 宏一览（共 9 个）

| 宏 | 生成 | 放哪 |
|---|---|---|
| `JM_DEFINE_JSON_INLINE(Type, fields...)` | to + from | 头文件（推荐默认） |
| `JM_DEFINE_JSON_TO_INLINE(Type, fields...)` | 仅 to | 头文件 |
| `JM_DEFINE_JSON_FROM_INLINE(Type, fields...)` | 仅 from | 头文件 |
| `JM_DECLARE_JSON(Type)` | 声明对 | 头文件（分离模式） |
| `JM_DEFINE_JSON(Type, fields...)` | to + from | 一个 cpp（分离模式） |
| `JM_DEFINE_JSON_TO(Type, fields...)` | 仅 to | 一个 cpp |
| `JM_DEFINE_JSON_FROM(Type, fields...)` | 仅 from | 一个 cpp |
| `JM_DEFINE_JSON_DERIVED_INLINE(Parent, Type, fields...)` | to + from（复用父类） | 头文件 |
| `JM_DEFINE_JSON_DERIVED(Parent, Type, fields...)` | 同上 | 一个 cpp |

- `TO` / `FROM` 是核心，`CODEC`（无后缀）是二者组合糖
- `DERIVED` 参数顺序为 `(Parent, Type, fields...)`，写反会触发 `static_assert`

## 反序列化语义

| 情况 | 行为 |
|---|---|
| 字段缺失 | 跳过，保留目标当前值（默认值） |
| 字段类型不匹配 | 抛 `jm::json::type_error` |
| 根不是 object | 抛 `jm::json::type_error` |

- 单字段校验（required 等）：建议 [json-schema-validator](https://github.com/pboettch/json-schema-validator) 或手写 `from_json`
- 错误诊断：debug 构建建议定义 `JSON_DIAGNOSTICS=1`，异常消息将携带 JSON Pointer
  （如 `(/items/1/name)`）。**注意：该宏带 ABI tag，必须对整个程序所有 TU 一致
  定义（建议挂 CMake 全局编译定义），不要只给单个 TU 定义。**

## 类型支持

- 基础类型、嵌套结构体：宏 + ADL 自动
- `std::vector` / `std::map` / `std::list` 等容器、`std::pair`（二元数组）：nlohmann 内置
- 枚举（`enum` / `enum class`）：nlohmann 内置按底层整数转换
  （`JSON_DISABLE_ENUM_SERIALIZATION` 默认启用）
- `std::optional`：C++17 及以上编译模式自动启用（按 `__cplusplus` / `_MSVC_LANG` 检测）
- 智能指针（`std::shared_ptr` / `std::unique_ptr`）：nlohmann 不内置，需要时手写 codec

## 构建与测试

```bash
# Linux
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j
ctest --test-dir build --output-on-failure
```

```powershell
# Windows
cmake -S . -B build
cmake --build build --config Debug
ctest --test-dir build -C Debug --output-on-failure
```

示例：

```powershell
.\build\Debug\example_header_only.exe
.\build\Debug\example_split.exe
```

```bash
./build/example_header_only
./build/example_split
```

## 离线与依赖

- `third_party/nlohmann/json.hpp`（v3.11.3）、`third_party/nlohmann/json_fwd.hpp`（由同一单头离线抽取）、`third_party/doctest/doctest.h`（v2.4.12）全部随仓库提交。
- 配置、构建、测试、示例不访问网络；不使用 FetchContent / ExternalProject / submodule / 包管理器。

## 版本管理

- Git 仓库以 `main` 为主分支，每个任务一个 Conventional Commit。
- `.gitattributes` 固定 `* text=auto eol=lf`，Windows/Linux checkout 行为一致。
- `build/`、`build-*/`、`cmake-build-*/`、`out/` 不入库。

## 使用方式

复制 `include/` 与 `third_party/` 入项目，或直接 `add_subdirectory` 后链接 `jm::jsonutils`。
