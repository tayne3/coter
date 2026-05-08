# coter

![CMake](https://img.shields.io/badge/CMake-3.14%2B-brightgreen?logo=cmake&logoColor=white)
[![Release](https://img.shields.io/github/v/release/tayne3/coter?include_prereleases&label=release&logo=github&logoColor=white)](https://github.com/tayne3/coter/releases)
[![Tag](https://img.shields.io/github/v/tag/tayne3/coter?color=%23ff8936&style=flat-square&logo=git&logoColor=white)](https://github.com/tayne3/coter/tags)
[![Ask DeepWiki](https://deepwiki.com/badge.svg)](https://deepwiki.com/tayne3/coter)

`coter` 是一个轻量级的 C 语言基础库。它将 C 语言开发中常用的基础设施进行了模块化封装，旨在提供一套接口清晰、性能稳定且易于集成的工具库。

`coter` 按功能划分为多个模块：

- **core**: 基础核心功能。
- **sync**: 并发同步原语。
- **container**: 常用容器与数据结构。
- **bytes**: 字节处理与缓存管理。
- **log**: 灵活的日志系统。
- **opt**: 命令行参数解析。
- **encoding**: 常用编解码工具。
- **crypto**: 基础加密算法。

## 🔨 项目集成

`coter` 使用 CMake 构建，可以通过 `FetchContent` 将其集成到你的工程中。

在你的 `CMakeLists.txt` 中添加：

```cmake
include(FetchContent)

FetchContent_Declare(
  coter
  GIT_REPOSITORY https://github.com/tayne3/coter.git
  GIT_TAG v0.10.4
)
FetchContent_MakeAvailable(coter)

target_link_libraries(your_project PRIVATE coter::coter)
```
