# coter

![CMake](https://img.shields.io/badge/CMake-3.14%2B-brightgreen?logo=cmake&logoColor=white)
[![Release](https://img.shields.io/github/v/release/tayne3/coter?include_prereleases&label=release&logo=github&logoColor=white)](https://github.com/tayne3/coter/releases)
[![Tag](https://img.shields.io/github/v/tag/tayne3/coter?color=%23ff8936&style=flat-square&logo=git&logoColor=white)](https://github.com/tayne3/coter/tags)
[![Ask DeepWiki](https://deepwiki.com/badge.svg)](https://deepwiki.com/tayne3/coter)

A lightweight, modular C foundation library. Each module is self-contained, has a minimal public API, and is built to be stable rather than feature-complete.

## Modules

| Module        | Description                                                                             |
| ------------- | --------------------------------------------------------------------------------------- |
| **core**      | Platform macros, strings, filesystem, time primitives, `expected`/`optional`            |
| **sync**      | Mutex, cond, rwlock, semaphore, atomic, event, waitgroup, message queue, pub/sub        |
| **thread**    | Thread create/join, thread pool, job pool, TLS, `once`                                  |
| **container** | Array, vector, list, stack, queue, priority queue, heap                                 |
| **bytes**     | Byte buffer, ring buffer, segmented buffer, buffer builder                              |
| **log**       | Async logger — console / rotating-file / custom handler, producer-consumer architecture |
| **time**      | Timer, ticker, cron scheduler, datetime formatting                                      |
| **ini**       | INI config file parser and serializer                                                   |
| **opt**       | Command-line argument parser                                                            |
| **encoding**  | Base64, hex, BCD, binary, JSON (bundled nlohmann/json)                                  |
| **crypto**    | MD5, SHA-1, generic hash interface                                                      |
| **fmt**       | C++ format support (bundled fmtlib)                                                     |

## Integration

`coter` uses CMake and can be consumed via `FetchContent`:

```cmake
include(FetchContent)

FetchContent_Declare(
  coter
  GIT_REPOSITORY https://github.com/tayne3/coter.git
  GIT_TAG        v0.11.1
)
FetchContent_MakeAvailable(coter)

target_link_libraries(your_project PRIVATE coter::coter)
```

## Build

```sh
cmake -B build
cmake --build build
```

To run tests:

```sh
cmake -B build -DCOTER_BUILD_TEST=ON
cmake --build build
ctest --test-dir build --output-on-failure
```
