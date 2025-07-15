# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

**coter** is a C99-compliant library providing infrastructure components for high-performance C projects. The library is organized into four main modules: base utilities, containers, mechanisms (threading/async), and algorithms.

## Build Commands

```bash
# Standard build
mkdir build && cd build
cmake ..
cmake --build .

# Build with tests and examples
cmake .. -DCOTER_BUILD_TEST=ON -DCOTER_BUILD_EXAMPLE=ON
cmake --build .

# Run tests
ctest

# Build with specific compiler
cmake .. -DCMAKE_C_COMPILER=clang

# Fast parallel build (CMake 3.12+)
cmake -S . -B build
cmake --build build --parallel 16
```

## Architecture

### Module Structure
- **base/**: Core utilities (platform abstraction, time, atomic operations)
- **container/**: Data structures (hash tables, lists, queues, vectors)
- **mech/**: Advanced mechanisms (thread pools, cron scheduler, event system, logging)
- **algo/**: Cryptographic and hashing algorithms (MD5, SHA1, Base64)

### Key Components
- **Thread Pool** (`ct_thpool`): Multi-threaded task execution
- **Cron Scheduler** (`ct_cron`): Unix cron-style job scheduling  
- **Event System** (`ct_event`): Asynchronous event handling
- **Memory Pools** (`ct_bytepool`): Efficient memory management
- **Ring Buffer** (`ct_rbuf`): Lock-free circular buffer

### Build System
- **CMake 3.14+** with modular configuration in `cmake/` directory
- **Build Options**: `COTER_BUILD_TEST`, `COTER_BUILD_EXAMPLE`, `COTER_BUILD_SHARED`, `COTER_BUILD_STATIC`
- **Output Targets**: `coter::obj`, `coter::shared`, `coter::static`, `coter::coter` (auto-alias)
- **Configuration**: Runtime config generated from `include/base/prefix/ct_config.h.in`

### Testing
- **Framework**: Custom CUnit implementation
- **Organization**: Tests mirror source structure (`test/algo/`, `test/base/`, etc.)
- **Execution**: Use `ctest` after building with `-DCOTER_BUILD_TEST=ON`

## Development Guidelines

### Code Standards
- **C99 compliance** required
- **Naming**: All public APIs use `ct_` prefix
- **Thread Safety**: Library designed for multi-threaded environments
- **Memory Management**: Careful allocation/deallocation patterns
- **Error Handling**: Comprehensive error checking and return codes

### Platform Support
- **Primary**: Linux, Windows, macOS
- **Conditional Compilation**: Extensive use of platform-specific `#ifdef` blocks
- **Dependencies**: pthread (with Windows fallback), standard C library

### Current Development
- **Main Branch**: `master` (for releases)
- **Development Branch**: `develop` (active development)
- **Recent Focus**: Cron scheduling and timer functionality improvements
- **CI/CD**: Gitea Actions with automated builds and tests