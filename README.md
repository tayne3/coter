
# coter 应用库


## 概述
coter是一个简化开发的C语言库，旨在为需要高性能和高可靠性的C语言项目提供强大的支持。


## 主要特性
- 轻量可靠: 轻量级实现，不占用过多资源, 并提供了完整的单元测试
- 模块化设计: 使用ANSI C(C99)标准编写, 无外部依赖, 各个模块间松耦合, 使用灵活
- 多种基础设施: 提供了多种基础设施的实现, 满足项目中常见的功能需求
- 简单易用: 提供简洁易用的API，降低开发者的维护成本和开发难度

## 构建和测试

coter 使用 CMake 作为构建系统，支持跨平台编译。

以下是一些常见平台上的构建示例：

### 在 Windows 平台上：

- 默认使用 Visual Studio 作为构建工具:

```sh
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022"
cmake --build . --parallel 16 --
```

- 指定 Ninja 作为构建工具, 指定 MinGW 编译器, 并编译示例程序:

```sh
mkdir build
cd build
cmake .. -G Ninja -DCMAKE_C_COMPILER="D:/SDK/MinGW/bin/gcc.exe" -DCTUNIT_BUILD_EXAMPLE=ON
cmake --build .
```

### 在 Linux 平台上：

- 默认使用 Makefiles 作为构建工具:

```sh
mkdir build
cd build
cmake ..
cmake --build .
```

- CMake 3.12+:

```sh
cmake -S . -B build
cmake --build build
```

- 指定 clang 编译器:

```sh
mkdir build
cd build
cmake .. -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++
cmake --build .
```


## 项目集成

### 1. 源代码集成

将 coter 项目的源代码直接添加到您的项目中，并在项目的构建系统中包含 coter 的源代码。
这种方式简单直接，适合需要对 coter 进行定制化修改的场景。

### 2. 输出库集成

您也可以将 coter 构建为静态库或动态库，然后在您的项目中链接这些库。
这种方式使得 coter 的更新和维护更加独立，便于管理。
