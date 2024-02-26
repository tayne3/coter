
# coter 应用库


## 概述
coter是一个简化开发的C语言库，旨在为需要高性能和高可靠性的C语言项目提供强大的支持。


## 主要特性
- 轻量可靠: 轻量级实现，不占用过多资源, 并提供了完整的单元测试
- 模块化设计: 使用ANSI C(C99)标准编写, 无外部依赖, 各个模块间松耦合, 使用灵活
- 多种基础设施: 提供了多种基础设施的实现, 满足项目中常见的功能需求
- 简单易用: 提供简洁易用的API，降低开发者的维护成本和开发难度

## 构建和测试

coter 使用 ctunit单元测试框架进行自动化测试, 构建和测试流程示例代码:

- 在Windows平台上, 使用Ninja作为构建工具:

```c
mkdir build
cd build
cmake .. -G Ninja
cmake --build .
ctest
```

- 在Windows平台上, 使用指定MinGW编译器:

```c
mkdir build
cd build
cmake .. -G "MinGW Makefiles" -DCMAKE_C_COMPILER="D:/SDK/MinGW/bin/gcc.exe"
cmake --build .
ctest
```

- 在Windows平台上, 使用Visual Studio作为构建工具:

```c
mkdir build
cd build
cmake ..
cmake --build . --config Release
ctest
```

- 在Linux平台上, 使用指定编译器:

```c
mkdir build
cd build
cmake ..
make -DCMAKE_C_COMPILER="/usr/bin/gcc"
make test
```

## 项目集成

### 1. 源代码集成

将 coter 项目的源代码直接拷贝至项目中进行使用。

### 2. 输出库集成

- 从 coter 仓库下载源码
- 编译形成共享库目标文件
- 将编译后的 lib 文件和头文件复制至当前项目的相应文件夹
- 在源文件中包含头文件并链接库文件





