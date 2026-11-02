# jm-jsonutils

给结构体加 JSON 序列化，通常需要手写一对 `to_json` / `from_json`。这个库将其简化为一行宏：

```cpp
#include <jm/json.h>

struct User
{
    std::string name;
    int age = 0;
};
JM_DEFINE_JSON_INLINE(User, name, age)   // 就这一行

User u{"tom", 18};
jm::json j = u;                 // {"age":18,"name":"tom"}
User v = j.get<User>();         // 反序列化
```

类型后面展开宏，然后当普通类型用——不需要注册，也不需要包一层封装。

底层是 [nlohmann/json](https://github.com/nlohmann/json) v3.11.3（版本锁死、随仓库提交）。容器、嵌套结构体、`std::pair` 等 nlohmann 已支持的类型，直接使用即可，无需额外编写代码：

```cpp
std::map<std::string, std::vector<User>> db;
jm::json jd = db;                                                      // 直接序列化
auto back = jd.get<std::map<std::string, std::vector<User>>>();        // 直接反序列化
```

- C++14 纯头文件，Windows（MSVC 2019+）/ Linux（GCC 9+、Clang 10+）
- `int64_t` 精确转换，不经过 double，不丢精度
- 完全离线：`third_party/` 入库，无 submodule / FetchContent / 包管理器

## 它做什么，不做什么

把字段逐个写进 JSON，再从 JSON 逐个读回来。支持继承、容器、嵌套、`std::optional`（C++17 起）。

不做的事：

- 自定义 key。key 永远是字段名本身，想改名字请手写 codec
- 必填、范围、格式等校验。建议用 [json-schema-validator](https://github.com/pboettch/json-schema-validator)，或手写 `from_json`
- 智能指针和多态类型。nlohmann 本身不内置，需要时手写

边界清楚，用起来就不容易踩空。

## 三种用法

### 1. 头文件内展开（推荐默认）

类型和宏都写在头文件里，宏名带 `_INLINE`：

```cpp
struct User
{
    std::string name;
    int age = 0;
};
JM_DEFINE_JSON_INLINE(User, name, age)
```

适用于绝大多数场景。生成的是 `inline` 函数，多个 TU 包含同一个头文件不会重复定义。

### 2. 声明与定义分离

当需要减少类型头文件对完整 nlohmann 的依赖时，可将声明与定义分离：

```cpp
// user_types.h —— 只放声明，引入轻量前向头
#include <string>
#include <jm/json_fwd.h>

struct Config
{
    std::string host;
    int port = 0;
};
JM_DECLARE_JSON(Config)
```

```cpp
// json_mappings.cpp —— 定义集中在一个 cpp
#include <jm/json.h>
#include "user_types.h"

JM_DEFINE_JSON(Config, host, port)
```

分离模式只生成双向 codec。若仅需 `to` 或 `from` 其中之一，请使用对应的 `_INLINE` 宏，或手写声明。

### 3. 继承

子类先复用父类的转换，再补充自身的字段：

```cpp
struct Base
{
    int id = 0;
};
JM_DEFINE_JSON_INLINE(Base, id)

struct Node : Base
{
    std::string tag;
};
JM_DEFINE_JSON_DERIVED_INLINE(Base, Node, tag)
```

结果是扁平 JSON：`{"id":1,"tag":"x"}`，父类字段不会嵌在一个子对象里。参数顺序是 `(Parent, Type, fields...)`，写反会触发 `static_assert`，报错信息会直接指出 first argument must be the base class。

## 宏一览（共 9 个）

| 宏 | 生成 | 放哪 |
|---|---|---|
| `JM_DEFINE_JSON_INLINE(Type, fields...)` | to + from | 头文件（推荐默认） |
| `JM_DEFINE_JSON_TO_INLINE(Type, fields...)` | 仅 to | 头文件 |
| `JM_DEFINE_JSON_FROM_INLINE(Type, fields...)` | 仅 from | 头文件 |
| `JM_DECLARE_JSON(Type)` | 声明对 | 类型头（分离模式） |
| `JM_DEFINE_JSON(Type, fields...)` | to + from | 一个 cpp（分离模式） |
| `JM_DEFINE_JSON_TO(Type, fields...)` | 仅 to | 一个 cpp |
| `JM_DEFINE_JSON_FROM(Type, fields...)` | 仅 from | 一个 cpp |
| `JM_DEFINE_JSON_DERIVED_INLINE(Parent, Type, fields...)` | to + from（复用父类） | 头文件 |
| `JM_DEFINE_JSON_DERIVED(Parent, Type, fields...)` | 同上 | 一个 cpp |

名字里的 `INLINE` 表示定义在头文件，没有 `INLINE` 的放 cpp。`TO` / `FROM` 是单向，不带后缀的是双向，等于把两者合在一起。

怎么选：

- 只想尽快能用 → `JM_DEFINE_JSON_INLINE`
- 想减少头文件引出的编译依赖 → `JM_DECLARE_JSON` + `JM_DEFINE_JSON`
- 有父类要复用 → `JM_DEFINE_JSON_DERIVED_INLINE`

## 几条规则

1. **宏必须写在类型所在的 namespace 里。** ADL 靠命名空间找到生成的函数；写在 namespace 外，nlohmann 的容器机制就找不到它们。
2. **key 固定为字段名**（`#field` 字符串化）。若需将 key 改为其他名称，或进行下划线转驼峰，宏无法满足，需手写 `to_json` / `from_json`。
3. **单次 1..63 个字段。** 空结构体不要用宏。字段太多的结构体，建议拆开。
4. **分离模式只支持双向 codec。** 单向请走 `_INLINE` 或手写声明。
5. **同一个程序不要混用 C++14 和 C++17 编译单元。** `std::optional` 的特化按 TU 的语言模式条件启用，混编会让不同 TU 看到不同的特化；nlohmann 本身也不建议混编。

## 反序列化行为

| 情况 | 行为 |
|---|---|
| 字段缺失 | 跳过，保留目标当前值（通常是默认值） |
| 字段类型不匹配 | 抛 `jm::json::type_error` |
| 根不是 object | 抛 `jm::json::type_error` |

字段缺失不报错是有意为之的设计：JSON 通常会增量演进，旧数据不应因字段增减而整体失败。也正因为如此，**为字段设置默认值很有必要**，缺字段时它将作为兜底。

想要更准确的错误定位，debug 构建可以定义 `JSON_DIAGNOSTICS=1`，异常消息会带 JSON Pointer（如 `(/items/1/name)`）。这个宏带 ABI tag，**必须对整个程序的所有 TU 一致定义**，建议挂在 CMake 全局编译定义上，不要只给单个 TU 加。

## 类型支持

| 类型 | 支持情况 |
|---|---|
| 基础类型、嵌套结构体 | 宏 + ADL 自动处理 |
| `std::vector` / `std::map` / `std::list`、`std::pair` | nlohmann 内置；`std::pair` 表示为二元数组 |
| `enum` / `enum class` | nlohmann 内置，按底层整数转换；定义 `JSON_DISABLE_ENUM_SERIALIZATION=1` 可关闭（关闭后本库的枚举字段也无法序列化） |
| `std::optional` | C++17 及以上编译模式自动启用；空值 ↔ `null` |
| `std::shared_ptr` / `std::unique_ptr` | nlohmann 不内置，需要时手写 codec |

`std::optional` 的启用按标准语言模式宏判断（`__cplusplus`、`_MSVC_LANG`，MSVC STL 下另含 `_HAS_CXX17`），不需要额外开关。

## 常见问题

**编译报找不到 `to_json` / `from_json`？**

先检查宏是否写在类型所在的 namespace 中，这是最常见的原因。若为分离模式，请进一步确认：声明与定义的签名是否一致，调用方是否包含了 `<jm/json.h>`（仅包含 `<jm/json_fwd.h>` 无法获得完整 JSON 类型）。

**字段缺失会丢数据吗？**

不会。缺失字段会保留目标对象当前值。`j.get<User>()` 会先默认构造，再逐字段覆盖。

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

运行示例：

```powershell
.\build\Debug\example_header_only.exe
.\build\Debug\example_split.exe
```

```bash
./build/example_header_only
./build/example_split
```

前提：CMake 3.20+，编译器满足前述平台要求。`/permissive-` 与 `/utf-8` 由库 target 自动设置，无需自行配置。

## 编辑器配置（clangd）

`Ctrl+Click` / `F12` 跳转由 clangd 提供，扩展已在 `.vscode/extensions.json` 中推荐。

clangd 可执行文件路径属于本机配置（通过用户设置 `clangd.path`、PATH，或由扩展提示下载），不入库；工作区只固定编译数据库的位置：

```powershell
# Windows：在 "Developer PowerShell for VS" 里执行；Linux 任意终端
cmake --preset clangd
```

## 离线与依赖

- `third_party/nlohmann/json.hpp`（v3.11.3）、`third_party/nlohmann/json_fwd.hpp`（由同一单头离线抽取）、`third_party/doctest/doctest.h`（v2.4.12）全部随仓库提交。
- 配置、构建、测试、示例都不访问网络；不使用 FetchContent / ExternalProject / submodule / 包管理器。

## 集成到项目

两种方式，任选其一：

1. 把 `include/` 与 `third_party/` 复制进项目，头文件路径指过去即可。
2. `add_subdirectory(jm-jsonutils)`，然后链接 `jm::jsonutils`。

推荐第二种，include 路径和编译选项（`/permissive-`、`/utf-8`）都由 target 自动传递。

## 版本管理

- Git 仓库以 `main` 为主分支，每个任务一个 Conventional Commit。
- `.gitattributes` 固定 `* text=auto eol=lf`，Windows / Linux checkout 行为一致。
- `build/`、`build-*/`、`cmake-build-*/`、`out/` 不入库。
