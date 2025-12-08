# 构建指南

## 🛠️ 先决条件

确保您的系统已安装以下工具：

- **C 编译器**（支持 C99 标准）
  - 推荐:
    - **GCC** 4.5+
    - **Clang** 10.0+
    - **MSVC** Visual Studio 2019+
    - **[Zig](https://ziglang.org/download/)** 0.15.0+
    - **[MinGW-w64](http://mingw-w64.org/doku.php/download)**（仅适用于 Windows）
- **[CMake](https://cmake.org/download/)** 版本 3.14+
- **[Ninja](https://github.com/ninja-build/ninja/releases)**（可选，用于高效构建）

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

#### 使用 CMake 3.14+ 更加简洁的语法:

```sh
cmake -B build
cmake --build build --parallel 16 --
```

- `-B build`：指定构建目录
- `--parallel 16`：启用16线程并行构建

## 🧪 运行测试

构建完成后，可以运行测试套件以验证库的功能：

```sh
cd build
ctest
```

- `ctest`：运行CMake测试套件
