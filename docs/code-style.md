# Coter 代码风格

状态：有效。

本文档是新代码和模块迁移期间的代码规范。尚未迁移的旧代码现状记录在
[迁移说明](migration.md) 中。

## 1. 语言与兼容性

- 库代码使用 **C99**。
- 公共头文件必须能被 **C++11** 翻译单元包含。
- C 头文件中的公共声明使用 `extern "C"` 包裹。
- 不使用 C11 及更新标准的语言特性，除非有明确的条件保护并写入文档。
- GNU 扩展和平台扩展只能出现在平台相关的实现文件中，公共头文件保持可移植。
- 优先使用 `<stdint.h>`、`<stddef.h>`、`<stdbool.h>` 中的标准类型。
- C 代码中的布尔值使用 `bool`，不要用 `int` 代替。

## 2. 命名

### 2.1 公共符号

所有公共 API 符号使用 `ct_` 前缀。内部跨文件辅助符号不属于公共 API，
不应出现在公共头文件或公共文档中。

| 类别 | 规范 | 示例 |
| --- | --- | --- |
| 公共函数 | `ct_<module>_<action>` | `ct_refcount_inc` |
| 公共类型 | `ct_<name>_t` | `ct_ref_t` |
| 结构体 tag | `ct_<name>` | `struct ct_ref` |
| 枚举类型 | `ct_<name>_t` | `ct_refcount_saturation_type_t` |
| 枚举常量 | `CT_<NAME>` | `CT_REFCOUNT_MAX` |
| 对象式宏 | `CT_<NAME>` | `CT_DATETIME_FMT_BUFLEN` |
| 公共访问宏 | `ct_<module>_<name>` | `ct_bytes_capacity` |
| 初始化/代码生成宏 | `CT_<NAME>` | `CT_WAITGROUP_INITIALIZER` |
| 公共回调类型 | `ct_<name>_t` | `ct_thpool_routine_t` |

函数名使用 `ct_<module>_<verb>[_<qualifier>]`：

- `ct_refcount_inc`
- `ct_datetime_add_days`
- `ct_msgqueue_push_for`
- `ct_bytes_is_empty`

以下后缀保持一致：

- `_for` 表示带超时的变体，例如 `ct_cond_wait_for`。
- `_is_` 或 `has_` 表示谓词，例如 `ct_bytes_is_empty`。
- `_and_` 表示名称中明确描述的多步复合操作，例如
  `ct_refcount_dec_and_test`。

不要引入新的随意缩写。已经存在的模块名缩写可以保留，例如
`msgqueue`、`thpool`、`rwlock`、`refcount`。

### 2.2 变量与参数

- 局部变量和函数参数使用 `snake_case`。
- 局部变量和函数参数不加后缀。
- 单字母名称只允许用于循环下标或含义明确的数学量。

```c
void ct_refcount_init(ct_refcount_t* ref) {
    uint32_t old;
    ...
}
```

### 2.3 结构体成员

- 结构体和类的数据成员使用 `snake_case_`，即**尾随下划线**。
- 该规则适用于公共结构体和内部结构体。
- 不要给局部变量或函数参数加尾随下划线。

```c
typedef struct ct_refcount {
    ct_refcount_value_t value_;
} ct_refcount_t;

typedef struct ct_ref {
    ct_refcount_t count_;
} ct_ref_t;
```

尾随下划线用于区分数据成员和普通局部变量、参数，同时避免前导下划线带来
的保留标识符和工具告警问题。

`sync/include/coter/sync/ref.h` 是这一规范的试点。

### 2.4 静态变量与全局变量

| 变量类型 | 规范 | 示例 |
| --- | --- | --- |
| 文件作用域 static 变量 | `s_<name>` | `s_refcount_mutex` |
| 函数内 static 变量 | `s_<name>` | `s_initialized` |
| 全局变量（外部链接） | `g_<name>` | `g_global_state` |
| 线程局部变量 | `tls_<name>` | `tls_cache` |

- `s_` 明确表示静态存储或文件内部状态。
- `g_` 只用于真正的全局状态。库代码应尽量避免可变全局状态，必须使用时
  要在注释中说明。
- `tls_` 用于线程局部存储，避免和普通全局变量混淆。
- `static const` 查询表也使用 `s_`：
  ```c
  static const char s_hex_encode_table[] = "0123456789abcdef";
  static const uint8_t s_days[12] = { ... };
  ```
- 不要使用前导下划线。

### 2.5 内部 static 函数

- 公开函数：`ct_<module>_<action>`
- 文件内 static 函数：`<module>_<action>`
- 不使用 `ct_` 前缀，不使用双下划线。

```c
/* 公共接口 */
ct_refcount_inc(...)
ct_msgqueue_push_for(...)

/* 内部 static 函数 */
refcount_dec_not_one(...)
msgqueue_wait_not_full(...)
datetime_valid_date(...)
```

这样可以通过名称区分公开接口和文件内部实现，同时避免 `__` 在 C++ 中属于
实现保留标识符的问题。

跨文件但仅在模块内部使用的函数，也放在 `src/` 下的私有头文件中，并使用
`<module>_<action>` 命名，不加 `CT_API`，不进入公开安装目录。

如果实现辅助函数因为公开头文件中的宏或 `static inline` 函数必须导出，按
2.9 使用实现 ABI 命名。

### 2.6 宏与常量

- 对象式宏、初始化宏和代码生成宏使用 `UPPER_SNAKE_CASE`。
- 公共对象式宏使用 `CT_` 前缀；公共函数式访问宏使用
  `ct_<module>_<name>`，见 2.1。
- 文件内宏可以不带 `CT_`，但必须保持大写，例如：
  ```c
  #define DATETIME_FMT "%04d-%02d-%02d ..."
  #define MAX_RETRY_COUNT 3
  ```
- 不使用 `kExampleValue` 形式的宏，也不引入 `k` 前缀常量风格。
- 有类型的 `static const` 对象使用 `s_<name>`，见 2.4。
- 枚举常量使用 `UPPER_SNAKE_CASE`；公共枚举常量加 `CT_` 前缀。
- 代码生成宏中的类型/名称元变量可以使用大写名称，例如 `TYPE`、`NAME`、
  `MEMBER`，因为它们会被替换进标识符，而不是作为普通值使用。
- 初始化宏使用 `CT_<NAME>` 形式，例如 `CT_BYTES_INIT`、`CT_BYTES_FROM`。

### 2.7 宏参数

- 普通值参数使用 `snake_case_` 尾随下划线。

```c
#define CT_MIN(a_, b_) ((a_) < (b_) ? (a_) : (b_))
#define CT_REF_DEFINE(NAME, TYPE, MEMBER) ...
```

### 2.8 文件与头文件保护

- 文件名使用 `snake_case`。
- 头文件扩展名为 `.h`，C 源文件为 `.c`，C++ 测试为 `.cpp`。
- 测试文件命名为 `<module>_test.cpp`。
- 头文件保护宏使用 `COTER_<PATH>_H`，例如 `COTER_SYNC_REF_H`。

### 2.9 公开头文件中的实现符号

公开头文件中的宏或 `static inline` 函数有时需要调用跨翻译单元的辅助函数。
这类符号不是稳定公共 API，但为了宏展开后的用户代码能够链接，必须导出。

| 类别 | 规范 | 示例 |
| --- | --- | --- |
| 实现 ABI | `ct_<module>_impl_<action>` | `ct_vector_impl_reserve` |

- 优先用宏或 `static inline` 消除这类符号；只有无法消除时才导出。
- 必须标注 `@internal`，并说明它不是稳定公共 API。
- 保留 `ct_` 前缀，不使用双下划线。
- 不进入公共文档的 API 列表；直接调用不承诺源码或二进制兼容性。

## 3. 头文件结构

公共头文件按以下顺序组织：

1. 简短文件注释。
2. Include guard。
3. 必需的头文件。
4. 面向 C++ 的 `extern "C"`。
5. 常量与类型。
6. 公共函数声明。
7. 结束 `extern "C"` 和 include guard。

规则：

- 公共头文件保持最小依赖，不包含实现专用头文件。
- 内部辅助函数和平台细节放在 `.c` 文件或 `src/` 下的私有头文件中。
- 导出函数使用 `CT_API`。
- 仅头文件的热路径或类型生成辅助函数使用 `CT_INLINE`。
- 忽略返回值可能造成危险时使用 `CT_ATTR_NODISCARD`。
- 所有权、生命周期和线程安全不明显时必须写明。
- 公共结构体字段属于源代码接口，重命名字段是源代码破坏性变更，即使内存
  布局没有变化。

## 4. 注释与文档

所有新增和迁移后的代码注释使用**英文**。`README_zh.md` 等面向中文用户的
文档不受此限制。

### 4.1 文件头

文件头保持简短：`@file`、`@brief`，最多再加一段短说明。不要把完整设计
论文写在文件头。

```c
/**
 * @file ref.h
 * @brief Saturating, thread-safe reference counting.
 *
 * Counters never wrap: invalid operations move them into a permanent poisoned
 * state. See ct_refcount_t for the state model.
 */
```

详细的不变量写在类型附近，实现原因写在实现附近。

### 4.2 类型注释

公共类型应说明：

- 状态与不变量。
- 所有权与生命周期。
- 线程安全保证。
- 调用方必须视为 opaque 的字段。

字段含义不直观时逐个说明。

### 4.3 函数注释

公共函数使用 Doxygen 标签：

- `@brief`
- `@param`
- `@return`
- `@note`
- `@see`
- 必要时使用 `@code`

只给真正不直观或所有权规则容易出错的 API 添加示例，不要给每个函数都加
示例。示例必须体现清理和失败处理。

### 4.4 实现注释

- 解释“为什么”，不要重复代码表面行为。
- 后端相关细节和扩展性限制写在实现文件中。
- 不保留注释掉的代码。
- 待办事项使用 `TODO(name): ...`。

## 5. 格式

格式以 `.clang-format` 和 `.editorconfig` 为准：

- 4 空格缩进，不使用 Tab。
- 最大行宽 120 列。
- UTF-8 编码，LF 换行，文件末尾保留换行。
- 大括号紧跟语句。
- 允许 clang-format 生成的短单行 `if`。
- Include 分组：自身头文件、标准头文件、项目头文件，组间空行。
- 由 clang-format 负责排序 include、对齐连续声明和赋值。

C 代码使用 `NULL`；C++ 测试代码可以使用 `nullptr`。

## 6. API 设计

- `_init` / `_destroy` 用于调用方拥有的对象。
- `_create` / `_destroy` 用于库分配的对象。
- 返回 `int` 的函数用 `0` 表示成功，非 `0` 表示错误码或失败。
- 返回 `bool` 的函数只表达布尔结果，不编码错误码。
- 返回指针的函数失败时返回 `NULL`。
- 校验公共输入，并在文档中写清失败行为。
- 明确内存由调用方还是被调用方拥有。
- 明确公共 API 的线程安全级别。
- 不要隐藏全局状态或全局锁；无法避免时必须文档化。

## 7. 测试

- 测试使用 doctest。
- 测试文件命名为 `<module>_test.cpp`。
- 测试套件使用模块名，例如 `TEST_SUITE_BEGIN("ref")`。
- 前置条件使用 `REQUIRE`，普通断言使用 `CHECK`。
- 测试名使用简短的小写描述。
- 测试必须确定性执行，不能依赖执行顺序。
- 并发测试使用有界的线程数和迭代数，不能无限等待。
- C++ 测试中的常量使用 `constexpr` 加 `snake_case`，例如
  `constexpr int thread_count = 8;`。不使用 `k` 前缀。

## 8. 相关文档

- [Git 提交风格](git-style.md)
- [迁移说明](migration.md)
