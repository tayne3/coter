# 构建指南

## 🛠️ 先决条件

确保您的系统已安装以下工具：

- **C 编译器**（支持 C99 标准）
  - 推荐版本：
    - **GCC** 4.5 或更高版本
    - **Clang** 10.0 或更高版本
    - **MSVC** Visual Studio 2019 或更高版本
- **CMake** 版本 3.05 及以上
  - 下载链接：[CMake 官方网站](https://cmake.org/download/)
- **Ninja**（可选，用于高效构建）
  - 下载链接：[Ninja Releases](https://github.com/ninja-build/ninja/releases)
- **MinGW-w64**（仅适用于 Windows，若使用 MinGW 编译）
  - 下载链接：[MinGW-w64](http://mingw-w64.org/doku.php/download)

## 📚 构建步骤

以下是针对不同平台和构建工具的详细构建步骤：

### 在 Windows 平台上：

#### 使用 Visual Studio 作为构建工具

1. 打开 **命令提示符** 或 **PowerShell**，导航到项目根目录：

    ```sh
    cd [project_root_dir]
    ```

2. 执行以下命令：

    ```sh
    mkdir build
    cd build
    cmake .. -G "Visual Studio 17 2022"
    cmake --build . --config Release
    ```

    - `mkdir build`：创建构建目录
    - `cmake .. -G "Visual Studio 17 2022"`：生成适用于 Visual Studio 2022 的解决方案文件
    - `cmake --build . --config Release`：编译项目的 Release 版本

#### 使用 Ninja 和 MinGW-w64

1. 确保已安装 Ninja 和 MinGW-w64，并将 Ninja 添加到系统环境变量中。

2. 打开 **命令提示符** 或 **PowerShell**，导航到项目根目录：

    ```sh
    cd [project_root_dir]
    ```

3. 执行以下命令：

    ```sh
    mkdir build
    cd build
    cmake .. -G Ninja -DCMAKE_C_COMPILER="D:/SDK/MinGW/bin/gcc.exe" -DCMAKE_BUILD_TYPE=Release
    cmake --build .
    ```

    - `-G Ninja`：指定使用 Ninja 作为构建系统
    - `-DCMAKE_C_COMPILER`：指定 MinGW-w64 的 GCC 编译器路径
    - `-DCMAKE_BUILD_TYPE=Release`：设置构建类型为 Release

### 在 Linux/macOS 平台上：

#### 使用默认构建工具（通常是 Makefiles）

```sh
mkdir build
cd build
cmake ..
make
```

- `cmake ..`：生成 Makefile
- `make`：编译项目

#### 指定 clang 编译器

```sh
mkdir build
cd build
cmake .. -DCMAKE_C_COMPILER=clang
make
```

- `-DCMAKE_C_COMPILER=clang`：指定使用 Clang 编译器

#### 使用交叉编译工具链

```sh
mkdir build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=../cmake/ToolchainLinux.cmake
make
```

- `-DCMAKE_TOOLCHAIN_FILE`：指定交叉编译工具链文件

#### 使用 CMake 3.12+ 更加简洁的语法:

```sh
cmake -S . -B build
cmake --build build --parallel 16 --
```

- `-S .`：指定源代码目录
- `-B build`：指定构建目录
- `--parallel 16`：启用16线程并行构建

## 🧪 运行测试

构建完成后，可以运行测试套件以验证库的功能：

```sh
cd build
ctest
```

- `ctest`：运行CMake测试套件

## ⚙️ 编译选项

- `COTER_BUILD_EXAMPLE`
    build example program (default OFF)
- `COTER_BUILD_TEST`
    build test program (default OFF)
- `COTER_BUILD_SHARED`
    build shared library (default ON)
- `COTER_BUILD_STATIC`
    build static library (default OFF)

## 🎯 编译目标

- **`coter::obj`**：对象库
  - 仅执行源代码的编译操作，不进行链接，**不会**生成任何可执行文件或库文件。
  - 从 CMake 3.12 版本开始，可以像使用普通库一样使用对象库。

- **`coter::shared`**：共享(动态)库
  - 构建为动态链接库，适用于需要在运行时加载或共享使用的场景。
  - 生成的库文件具有 `.dll`（Windows）、`.so`（Linux）或 `.dylib`（macOS）扩展名。

- **`coter::static`**：静态库
  - 构建为静态链接库，适用于需要将库静态链接到最终可执行文件中的场景。
  - 生成的库文件具有 `.lib`（Windows）或 `.a`（Linux/macOS）扩展名。

- **`coter::coter`**：通用库目标
  - 根据构建配置自动别名化为共享库、静态库或对象库：
    - 启用 `COTER_BUILD_SHARED`：
        - `coter::coter` 将指向 `coter::shared`。
    - 启用 `COTER_BUILD_STATIC`：
        - `coter::coter` 将指向 `coter::static`。
    - 同时启用 `COTER_BUILD_SHARED` 和 `COTER_BUILD_STATIC`：
        - `coter::coter` 优先指向 `coter::shared`。
    - 未启用 `COTER_BUILD_SHARED` 和 `COTER_BUILD_STATIC`：
        - `coter::coter` 将指向 `coter::obj`。

## 🔗 项目集成

### 子模块集成

    - 将 coter 项目作为 CMake 子模块直接添加到您的项目中。这样，您的项目可以直接使用 coter 的源码，并且能够方便地跟踪和更新 coter 的版本。

### 输出库集成

    - 您也可以将 coter 构建为静态库或动态库，然后在您的项目中链接这些库。这种方式使得 coter 的更新和维护更加独立，便于管理。
