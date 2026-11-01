# coter

![CMake](https://img.shields.io/badge/CMake-3.14%2B-brightgreen?logo=cmake&logoColor=white)
[![Release](https://img.shields.io/github/v/release/tayne3/coter?include_prereleases&label=release&logo=github&logoColor=white)](https://github.com/tayne3/coter/releases)
[![Tag](https://img.shields.io/github/v/tag/tayne3/coter?color=%23ff8936&style=flat-square&logo=git&logoColor=white)](https://github.com/tayne3/coter/tags)
[![Ask DeepWiki](https://deepwiki.com/badge.svg)](https://deepwiki.com/tayne3/coter)

一个轻量级的模块化 C 语言基础库。每个模块独立自洽、公开接口极少，以稳定可靠为首要目标，而非追求功能的全面性。

## 模块

| 模块          | 说明                                                                                            |
| ------------- | ----------------------------------------------------------------------------------------------- |
| **core**      | 平台宏、字符串、文件系统、时间、随机数                                                   |
| **sync**      | mutex、cond、rwlock、semaphore、atomic、引用计数、event、waitgroup、消息队列、发布订阅、关闭信号 |
| **thread**    | 线程创建与等待、线程池、任务池、TLS、`once`                                                     |
| **container** | array、vector、list、stack、queue、优先队列、heap                                               |
| **bytes**     | 字节缓冲、环形缓冲、消息缓冲                                                                    |
| **opt**       | 命令行参数解析与帮助格式化                                                                      |
| **encoding**  | Base64、Hex、BCD、二进制                                                                        |
| **crypto**    | MD5、SHA-1、常见哈希算法                                                                        |

## 集成

`coter` 使用 CMake 构建，可通过 `FetchContent` 集成到你的项目：

```cmake
include(FetchContent)

FetchContent_Declare(
  coter
  GIT_REPOSITORY https://github.com/tayne3/coter.git
  GIT_TAG        v0.13.0
)
FetchContent_MakeAvailable(coter)

target_link_libraries(your_project PRIVATE coter::coter)
```

## 构建

```sh
cmake -B build
cmake --build build
```

运行测试：

```sh
cmake -B build -DCOTER_BUILD_TESTS=ON
cmake --build build
ctest --test-dir build --output-on-failure
```
