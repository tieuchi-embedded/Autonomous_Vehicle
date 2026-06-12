# Hướng Dẫn CMake Chi Tiết - Từ Cơ Bản Đến Nâng Cao

## Mục Lục

1. [Giới thiệu CMake](#1-giới-thiệu-cmake)
2. [Cài đặt CMake](#2-cài-đặt-cmake)
3. [Khái niệm cơ bản](#3-khái-niệm-cơ-bản)
4. [Cú pháp CMakeLists.txt](#4-cú-pháp-cmakeliststxt)
5. [Các lệnh built-in thường dùng](#5-các-lệnh-built-in-thường-dùng)
6. [Biến và thuộc tính](#6-biến-và-thuộc-tính)
7. [Cách build với CMake](#7-cách-build-với-cmake)
8. [Các file được tạo ra khi build](#8-các-file-được-tạo-ra-khi-build)
9. [Ví dụ từ cơ bản đến nâng cao](#9-ví-dụ-từ-cơ-bản-đến-nâng-cao)
10. [Best Practices](#10-best-practices)
11. [Troubleshooting](#11-troubleshooting)

---

## 1. Giới Thiệu CMake

**CMake** (Cross-platform Make) là công cụ build system mã nguồn mở, dùng để **tự động hóa quá trình biên dịch** phần mềm trên nhiều nền tảng khác nhau.

### CMake làm gì?

CMake **không phải compiler** — nó là một **meta build system**:
- Đọc file cấu hình `CMakeLists.txt`
- Sinh ra file build phù hợp với hệ điều hành (Makefile, Visual Studio project, Ninja...)
- Gọi compiler thật (gcc, clang, MSVC) để biên dịch

### Tại sao dùng CMake?

| Ưu điểm | Mô tả |
|---------|-------|
| **Đa nền tảng** | Một file `CMakeLists.txt` → build được trên Windows, Linux, macOS |
| **Quản lý dependencies** | Tự động tìm và link thư viện |
| **Tiêu chuẩn công nghiệp** | Hầu hết dự án C/C++ lớn đều dùng (LLVM, Qt, OpenCV, KDE...) |
| **Tích hợp IDE** | Hỗ trợ CLion, VS Code, Visual Studio, Qt Creator... |

### Luồng hoạt động

```
┌─────────────────┐    cmake ..    ┌─────────────┐  cmake --build  ┌─────────────┐
│ CMakeLists.txt  │ ─────────────► │  Makefile   │ ──────────────► │ Executable  │
│  (bạn viết)     │                │ (generated) │                  │  (output)   │
└─────────────────┘                └─────────────┘                  └─────────────┘
```

---

## 2. Cài Đặt CMake

### Linux (Ubuntu/Debian)
```bash
sudo apt update
sudo apt install cmake
```

### Linux (Fedora/RHEL)
```bash
sudo dnf install cmake
```

### macOS
```bash
# Dùng Homebrew
brew install cmake
```

### Windows
- Download tại: https://cmake.org/download/
- Hoặc dùng Chocolatey:
```powershell
choco install cmake
```

### Kiểm tra cài đặt
```bash
cmake --version
```

Output mong đợi:
```
cmake version 3.28.x
```

> **Khuyến nghị:** Dùng CMake **>= 3.15** để tận dụng đầy đủ các tính năng hiện đại.

---

## 3. Khái Niệm Cơ Bản

### 3.1. CMakeLists.txt

File chứa **toàn bộ cấu hình build** của project. Mỗi project có ít nhất 1 file `CMakeLists.txt` ở thư mục gốc.

### 3.2. Target

**Target** là đối tượng được tạo ra khi build, gồm 3 loại:

| Loại Target | Lệnh tạo | Mô tả |
|-------------|----------|-------|
| **Executable** | `add_executable()` | File chạy được (.exe, binary) |
| **Library** | `add_library()` | Thư viện (.a, .so, .lib, .dll) |
| **Custom** | `add_custom_target()` | Target tùy chỉnh (chạy script, copy file...) |

### 3.3. Source vs Build directory

```
my_project/              ← Source directory (chứa code + CMakeLists.txt)
├── CMakeLists.txt
├── main.cpp
└── build/               ← Build directory (CHỨA file generated)
    ├── Makefile
    └── MyApp
```

> **Quy tắc quan trọng:** LUÔN tạo thư mục `build/` riêng, không build trong source directory ("out-of-source build").

### 3.4. Generator

**Generator** quyết định CMake sinh ra loại build file gì:

| Generator | OS | Output |
|-----------|-----|--------|
| `Unix Makefiles` | Linux/macOS | Makefile |
| `Ninja` | Mọi OS | build.ninja (nhanh hơn Make) |
| `Visual Studio 17 2022` | Windows | .sln, .vcxproj |
| `Xcode` | macOS | .xcodeproj |

---

## 4. Cú Pháp CMakeLists.txt

### 4.1. Cú pháp cơ bản

```cmake
command_name(arg1 arg2 arg3)
```

- **Không phân biệt hoa thường** ở tên lệnh (`set` = `SET` = `Set`), nhưng quy ước viết **chữ thường**.
- **Tham số** ngăn cách bởi khoảng trắng hoặc xuống dòng.
- **Comment** bắt đầu bằng `#`.

### 4.2. Template chuẩn

```cmake
# 1. Version CMake tối thiểu (BẮT BUỘC ở đầu file)
cmake_minimum_required(VERSION 3.15)

# 2. Khai báo project
project(MyApp 
    VERSION 1.0.0
    DESCRIPTION "My Application"
    LANGUAGES CXX
)

# 3. Cài đặt chuẩn C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 4. Tìm thư viện ngoài (nếu có)
find_package(Threads REQUIRED)

# 5. Tạo target
add_executable(MyApp main.cpp src/foo.cpp)

# 6. Include directories
target_include_directories(MyApp PRIVATE include/)

# 7. Link libraries
target_link_libraries(MyApp PRIVATE Threads::Threads)
```

---

## 5. Các Lệnh Built-in Thường Dùng

### 5.1. Lệnh khởi tạo (BẮT BUỘC)

#### `cmake_minimum_required`
Khai báo version CMake tối thiểu.
```cmake
cmake_minimum_required(VERSION 3.15)
```

#### `project`
Khai báo tên và metadata project.
```cmake
project(MyApp 
    VERSION 1.0.0           # Version
    DESCRIPTION "..."       # Mô tả
    LANGUAGES CXX C         # Ngôn ngữ (CXX = C++, C, Fortran...)
)
```

### 5.2. Tạo Target

#### `add_executable`
Tạo file thực thi.
```cmake
add_executable(target_name source1.cpp source2.cpp ...)

# Ví dụ
add_executable(MyApp main.cpp utils.cpp)
```

#### `add_library`
Tạo thư viện.
```cmake
# Static library (.a / .lib)
add_library(MyLib STATIC src/foo.cpp src/bar.cpp)

# Shared library (.so / .dll)
add_library(MyLib SHARED src/foo.cpp src/bar.cpp)

# Header-only library
add_library(MyLib INTERFACE)
```

### 5.3. Lệnh target-based (Modern CMake)

#### `target_include_directories`
Thêm include path cho 1 target cụ thể.
```cmake
target_include_directories(MyApp 
    PRIVATE include/        # Chỉ MyApp dùng
    PUBLIC public_headers/  # MyApp + những target link với MyApp dùng
    INTERFACE api/          # Chỉ những target link với MyApp dùng
)
```

| Phạm vi | Khi nào dùng |
|---------|-------------|
| `PRIVATE` | Chỉ target hiện tại dùng |
| `PUBLIC` | Cả target hiện tại + target khác link với nó |
| `INTERFACE` | Chỉ target khác link với nó (dùng cho header-only lib) |

#### `target_link_libraries`
Link target với thư viện.
```cmake
target_link_libraries(MyApp 
    PRIVATE 
        MyLib
        Threads::Threads
        OpenSSL::SSL
)
```

#### `target_compile_definitions`
Định nghĩa macro cho target.
```cmake
target_compile_definitions(MyApp PRIVATE DEBUG_MODE=1 VERSION="1.0")
# Tương đương -DDEBUG_MODE=1 -DVERSION="1.0" khi compile
```

#### `target_compile_options`
Thêm flag cho compiler.
```cmake
target_compile_options(MyApp PRIVATE -Wall -Wextra -O2)
```

#### `target_compile_features`
Yêu cầu tính năng C++.
```cmake
target_compile_features(MyApp PRIVATE cxx_std_17)
```

### 5.4. Tìm thư viện

#### `find_package`
Tìm thư viện đã cài trong hệ thống.
```cmake
find_package(OpenSSL REQUIRED)        # Bắt buộc phải có
find_package(Boost 1.70 COMPONENTS system filesystem)
find_package(Qt6 COMPONENTS Core Widgets REQUIRED)
```

#### `find_library` / `find_path`
Tìm file thư viện hoặc header thủ công.
```cmake
find_library(MATH_LIB m)              # Tìm libm
find_path(JSON_HEADER nlohmann/json.hpp)
```

### 5.5. Quản lý subdirectory

#### `add_subdirectory`
Gọi CMakeLists.txt trong thư mục con.
```cmake
add_subdirectory(src)
add_subdirectory(tests)
add_subdirectory(third_party/googletest)
```

### 5.6. Biến và logic

#### `set` / `unset`
```cmake
set(VAR_NAME "value")                # Tạo biến
set(SOURCES main.cpp utils.cpp)      # List
set(CMAKE_CXX_STANDARD 17)           # Override biến CMake
unset(VAR_NAME)                      # Xóa biến
```

#### `option`
Tạo option bật/tắt (ON/OFF).
```cmake
option(BUILD_TESTS "Build unit tests" ON)
option(USE_OPENMP "Enable OpenMP" OFF)

# Sử dụng:
if(BUILD_TESTS)
    add_subdirectory(tests)
endif()
```

Bật/tắt khi build:
```bash
cmake -DBUILD_TESTS=OFF ..
```

#### `if / elseif / else / endif`
```cmake
if(WIN32)
    message("Building on Windows")
elseif(APPLE)
    message("Building on macOS")
elseif(UNIX)
    message("Building on Linux")
endif()

# Kiểm tra biến
if(DEFINED MY_VAR)
    message("MY_VAR is defined")
endif()

# So sánh
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message("Debug mode")
endif()
```

#### `foreach / endforeach`
```cmake
set(FILES a.cpp b.cpp c.cpp)
foreach(FILE ${FILES})
    message("Processing: ${FILE}")
endforeach()
```

#### `function` / `macro`
```cmake
function(print_hello name)
    message("Hello, ${name}!")
endfunction()

print_hello("World")    # Output: Hello, World!
```

### 5.7. File và đường dẫn

#### `file`
```cmake
# Glob files (KHÔNG khuyến nghị cho source files)
file(GLOB SOURCES "src/*.cpp")
file(GLOB_RECURSE ALL_SOURCES "src/*.cpp")

# Tạo/đọc file
file(WRITE output.txt "Hello")
file(READ input.txt CONTENT)

# Copy file
file(COPY config.json DESTINATION ${CMAKE_BINARY_DIR})
```

#### `configure_file`
Tạo file từ template, thay biến `@VAR@` hoặc `${VAR}`.
```cmake
# config.h.in
# #define VERSION "@PROJECT_VERSION@"

configure_file(config.h.in config.h)
```

### 5.8. Cài đặt (Install)

#### `install`
```cmake
# Cài executable
install(TARGETS MyApp DESTINATION bin)

# Cài library
install(TARGETS MyLib 
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

# Cài header
install(DIRECTORY include/ DESTINATION include)

# Cài file đơn lẻ
install(FILES README.md DESTINATION share/myapp)
```

### 5.9. Testing

#### `enable_testing` / `add_test`
```cmake
enable_testing()

add_executable(MyTest test.cpp)
add_test(NAME MyTest COMMAND MyTest)
```

### 5.10. Thông báo

#### `message`
```cmake
message(STATUS "Building MyApp v${PROJECT_VERSION}")    # Thông tin
message(WARNING "This is a warning")                    # Cảnh báo
message(FATAL_ERROR "Cannot find required library")     # Lỗi (dừng cmake)
```

---

## 6. Biến và Thuộc Tính

### 6.1. Biến tự động của CMake

| Biến | Ý nghĩa |
|------|---------|
| `CMAKE_SOURCE_DIR` | Thư mục gốc chứa CMakeLists.txt |
| `CMAKE_BINARY_DIR` | Thư mục build |
| `CMAKE_CURRENT_SOURCE_DIR` | Thư mục source hiện tại |
| `CMAKE_CURRENT_BINARY_DIR` | Thư mục build hiện tại |
| `PROJECT_NAME` | Tên project (từ `project()`) |
| `PROJECT_VERSION` | Version (1.0.0) |
| `PROJECT_VERSION_MAJOR` | Major version (1) |
| `CMAKE_CXX_COMPILER` | Đường dẫn compiler |
| `CMAKE_BUILD_TYPE` | Debug/Release/RelWithDebInfo/MinSizeRel |

### 6.2. Biến nền tảng

```cmake
if(WIN32)         # Windows
if(UNIX)          # Linux + macOS
if(APPLE)         # macOS
if(LINUX)         # Linux (CMake 3.25+)
if(MSVC)          # Microsoft compiler
if(MINGW)         # MinGW
```

### 6.3. Biến cấu hình build

| Biến | Mục đích |
|------|----------|
| `CMAKE_CXX_STANDARD` | Chuẩn C++ (11, 14, 17, 20, 23) |
| `CMAKE_CXX_STANDARD_REQUIRED` | Bắt buộc dùng đúng chuẩn (ON/OFF) |
| `CMAKE_CXX_EXTENSIONS` | Cho phép GNU extension (OFF khuyến nghị) |
| `CMAKE_BUILD_TYPE` | Debug/Release |
| `CMAKE_INSTALL_PREFIX` | Đường dẫn install (default: /usr/local) |

### 6.4. Cách dùng biến

```cmake
set(NAME "World")
message("Hello ${NAME}")              # Hello World

# Với list
set(FILES a.cpp b.cpp c.cpp)
message("${FILES}")                   # a.cpp;b.cpp;c.cpp
```

---

## 7. Cách Build Với CMake

### 7.1. Build cơ bản (3 bước)

```bash
# Bước 1: Tạo thư mục build
mkdir build && cd build

# Bước 2: Configure (generate Makefile)
cmake ..

# Bước 3: Compile
cmake --build .
```

### 7.2. Cú pháp đầy đủ

```bash
cmake [options] <source-dir>
cmake --build <build-dir> [options]
```

### 7.3. Các option khi configure

```bash
# Chỉ định build type
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake -DCMAKE_BUILD_TYPE=Debug ..

# Chỉ định generator
cmake -G "Ninja" ..
cmake -G "Visual Studio 17 2022" ..

# Truyền biến tùy chỉnh
cmake -DBUILD_TESTS=ON -DUSE_OPENMP=ON ..

# Chỉ định compiler
cmake -DCMAKE_CXX_COMPILER=clang++ ..

# Chỉ định install prefix
cmake -DCMAKE_INSTALL_PREFIX=/opt/myapp ..

# Verbose
cmake --debug-output ..
```

### 7.4. Các option khi build

```bash
# Build song song (nhanh hơn)
cmake --build . --parallel
cmake --build . -j 8                 # Dùng 8 cores

# Build target cụ thể
cmake --build . --target MyApp

# Clean rồi build
cmake --build . --target clean
cmake --build . --clean-first

# Verbose (xem lệnh compiler)
cmake --build . --verbose
```

### 7.5. Cú pháp hiện đại (CMake 3.13+)

```bash
# Một lệnh configure
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build --parallel

# Install
cmake --install build --prefix /usr/local

# Test
ctest --test-dir build
```

### 7.6. Ví dụ luồng đầy đủ

```bash
# Clone code
git clone <repo>
cd <repo>

# Configure
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build --parallel

# Test
ctest --test-dir build

# Install
sudo cmake --install build

# Chạy
./build/MyApp
```

---

## 8. Các File Được Tạo Ra Khi Build

### 8.1. Cấu trúc thư mục build sau khi `cmake ..`

```
build/
├── CMakeCache.txt           # Cache các biến cấu hình (giữ giữa các lần build)
├── CMakeFiles/              # Thư mục nội bộ của CMake
│   ├── CMakeCXXCompiler.cmake
│   ├── CMakeOutput.log
│   └── 3.x.x/               # Files theo version CMake
├── cmake_install.cmake      # Script install
└── Makefile                 # File build chính (Linux/macOS)
```

### 8.2. Cấu trúc sau khi `cmake --build .`

```
build/
├── CMakeCache.txt
├── CMakeFiles/
│   └── MyApp.dir/           # Object files của target MyApp
│       ├── main.cpp.o       # Object file (đã compile)
│       └── utils.cpp.o
├── Makefile
└── MyApp                    # ✅ Executable cuối cùng
```

### 8.3. Các loại file output

| File | Mô tả | OS |
|------|-------|-----|
| `MyApp` / `MyApp.exe` | Executable | Linux/Win |
| `libMyLib.a` | Static library | Linux/macOS |
| `libMyLib.so` | Shared library | Linux |
| `libMyLib.dylib` | Shared library | macOS |
| `MyLib.lib` | Static library | Windows |
| `MyLib.dll` | Shared library | Windows |
| `*.o` / `*.obj` | Object file (intermediate) | Linux/Win |

### 8.4. File quan trọng cần biết

| File | Mục đích |
|------|----------|
| `CMakeCache.txt` | Lưu biến cấu hình. **Xóa file này = reset config** |
| `compile_commands.json` | Database lệnh compile (cho clangd, IDE) |
| `install_manifest.txt` | Danh sách file đã install |

> **Mẹo:** Để tạo `compile_commands.json`:
> ```cmake
> set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
> ```

---

## 9. Ví Dụ Từ Cơ Bản Đến Nâng Cao

### Ví dụ 1: Hello World (Cơ bản)

**Cấu trúc:**
```
hello/
├── CMakeLists.txt
└── main.cpp
```

**main.cpp:**
```cpp
#include <iostream>
int main() {
    std::cout << "Hello, CMake!\n";
    return 0;
}
```

**CMakeLists.txt:**
```cmake
cmake_minimum_required(VERSION 3.15)
project(Hello)
add_executable(hello main.cpp)
```

**Build & Run:**
```bash
cmake -S . -B build
cmake --build build
./build/hello
```

---

### Ví dụ 2: Project nhiều file

**Cấu trúc:**
```
calc/
├── CMakeLists.txt
├── main.cpp
├── include/
│   └── math_utils.h
└── src/
    └── math_utils.cpp
```

**math_utils.h:**
```cpp
#pragma once
int add(int a, int b);
int multiply(int a, int b);
```

**math_utils.cpp:**
```cpp
#include "math_utils.h"
int add(int a, int b) { return a + b; }
int multiply(int a, int b) { return a * b; }
```

**main.cpp:**
```cpp
#include <iostream>
#include "math_utils.h"
int main() {
    std::cout << "2 + 3 = " << add(2, 3) << "\n";
    std::cout << "2 * 3 = " << multiply(2, 3) << "\n";
}
```

**CMakeLists.txt:**
```cmake
cmake_minimum_required(VERSION 3.15)
project(Calc VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(calc 
    main.cpp 
    src/math_utils.cpp
)

target_include_directories(calc PRIVATE include)
```

---

### Ví dụ 3: Tạo và dùng Library

**Cấu trúc:**
```
project/
├── CMakeLists.txt
├── app/
│   ├── CMakeLists.txt
│   └── main.cpp
└── mathlib/
    ├── CMakeLists.txt
    ├── include/
    │   └── mathlib/math.h
    └── src/
        └── math.cpp
```

**Root `CMakeLists.txt`:**
```cmake
cmake_minimum_required(VERSION 3.15)
project(MyProject VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)

# Add các subdirectory
add_subdirectory(mathlib)
add_subdirectory(app)
```

**`mathlib/CMakeLists.txt`:**
```cmake
add_library(mathlib STATIC src/math.cpp)

target_include_directories(mathlib 
    PUBLIC include      # Header public cho user dùng
)
```

**`app/CMakeLists.txt`:**
```cmake
add_executable(app main.cpp)

target_link_libraries(app PRIVATE mathlib)
```

**Build:**
```bash
cmake -S . -B build
cmake --build build
./build/app/app
```

---

### Ví dụ 4: Dùng thư viện ngoài (find_package)

```cmake
cmake_minimum_required(VERSION 3.15)
project(WebApp)

set(CMAKE_CXX_STANDARD 17)

# Tìm thư viện
find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)
find_package(Boost 1.70 REQUIRED COMPONENTS system filesystem)

add_executable(webapp main.cpp)

target_link_libraries(webapp 
    PRIVATE
        OpenSSL::SSL
        OpenSSL::Crypto
        Threads::Threads
        Boost::system
        Boost::filesystem
)
```

---

### Ví dụ 5: Conditional compile + Options

```cmake
cmake_minimum_required(VERSION 3.15)
project(MyApp)

set(CMAKE_CXX_STANDARD 17)

# Tạo các option
option(BUILD_TESTS "Build unit tests" ON)
option(ENABLE_LOGGING "Enable logging" OFF)
option(USE_MULTITHREADING "Use threads" ON)

add_executable(myapp main.cpp)

# Conditional compile definitions
if(ENABLE_LOGGING)
    target_compile_definitions(myapp PRIVATE ENABLE_LOG=1)
endif()

# Platform-specific
if(WIN32)
    target_compile_definitions(myapp PRIVATE PLATFORM_WIN=1)
    target_link_libraries(myapp PRIVATE ws2_32)
elseif(UNIX)
    target_compile_definitions(myapp PRIVATE PLATFORM_UNIX=1)
endif()

# Thread support
if(USE_MULTITHREADING)
    find_package(Threads REQUIRED)
    target_link_libraries(myapp PRIVATE Threads::Threads)
endif()

# Build tests
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
```

**Build với options:**
```bash
cmake -S . -B build -DBUILD_TESTS=OFF -DENABLE_LOGGING=ON
cmake --build build
```

---

### Ví dụ 6: Compile flags theo build type

```cmake
cmake_minimum_required(VERSION 3.15)
project(MyApp)

set(CMAKE_CXX_STANDARD 17)

add_executable(myapp main.cpp)

# Flags chung
target_compile_options(myapp PRIVATE
    -Wall -Wextra -Wpedantic
)

# Flags riêng theo build type
target_compile_options(myapp PRIVATE
    $<$<CONFIG:Debug>:-g -O0 -DDEBUG>
    $<$<CONFIG:Release>:-O3 -DNDEBUG>
)

# Hoặc dùng if
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(myapp PRIVATE DEBUG_MODE)
endif()
```

---

### Ví dụ 7: Generate config file

**Cấu trúc:**
```
project/
├── CMakeLists.txt
├── config.h.in
└── main.cpp
```

**config.h.in:**
```cpp
#pragma once
#define PROJECT_NAME "@PROJECT_NAME@"
#define PROJECT_VERSION "@PROJECT_VERSION@"
#define BUILD_DATE "@BUILD_DATE@"
#cmakedefine ENABLE_FEATURE_X
```

**CMakeLists.txt:**
```cmake
cmake_minimum_required(VERSION 3.15)
project(MyApp VERSION 2.5.1)

option(ENABLE_FEATURE_X "Enable feature X" ON)

string(TIMESTAMP BUILD_DATE "%Y-%m-%d")

# Generate config.h từ config.h.in
configure_file(
    config.h.in
    ${CMAKE_BINARY_DIR}/generated/config.h
)

add_executable(myapp main.cpp)
target_include_directories(myapp PRIVATE ${CMAKE_BINARY_DIR}/generated)
```

**main.cpp:**
```cpp
#include <iostream>
#include "config.h"

int main() {
    std::cout << PROJECT_NAME << " v" << PROJECT_VERSION << "\n";
    std::cout << "Built on " << BUILD_DATE << "\n";
#ifdef ENABLE_FEATURE_X
    std::cout << "Feature X is enabled\n";
#endif
}
```

---

### Ví dụ 8: Project hoàn chỉnh với Tests + Install

**Cấu trúc:**
```
project/
├── CMakeLists.txt
├── include/myapp/
│   └── calculator.h
├── src/
│   ├── calculator.cpp
│   └── main.cpp
└── tests/
    ├── CMakeLists.txt
    └── test_calculator.cpp
```

**Root CMakeLists.txt:**
```cmake
cmake_minimum_required(VERSION 3.15)
project(MyApp 
    VERSION 1.0.0
    DESCRIPTION "Professional CMake Example"
    LANGUAGES CXX
)

# Cài đặt chung
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile commands cho IDE
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Default build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Options
option(BUILD_TESTS "Build tests" ON)
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)

# Tạo library
add_library(calculator src/calculator.cpp)
target_include_directories(calculator PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_compile_features(calculator PUBLIC cxx_std_17)

# Tạo executable
add_executable(myapp src/main.cpp)
target_link_libraries(myapp PRIVATE calculator)

# Compile warnings
if(MSVC)
    target_compile_options(calculator PRIVATE /W4)
    target_compile_options(myapp PRIVATE /W4)
else()
    target_compile_options(calculator PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(myapp PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Tests
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Install
include(GNUInstallDirs)

install(TARGETS calculator myapp
    EXPORT MyAppTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(DIRECTORY include/ 
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Print info
message(STATUS "Project: ${PROJECT_NAME} v${PROJECT_VERSION}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Build tests: ${BUILD_TESTS}")
```

**tests/CMakeLists.txt:**
```cmake
add_executable(test_calc test_calculator.cpp)
target_link_libraries(test_calc PRIVATE calculator)

add_test(NAME CalculatorTest COMMAND test_calc)
```

**Build & test:**
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
sudo cmake --install build --prefix /usr/local
```

---

### Ví dụ 9: FetchContent (download library tự động)

```cmake
cmake_minimum_required(VERSION 3.15)
project(MyApp)

set(CMAKE_CXX_STANDARD 17)

# Tự động download và build thư viện
include(FetchContent)

FetchContent_Declare(
    fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG 10.1.1
)

FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
)

FetchContent_MakeAvailable(fmt json)

add_executable(myapp main.cpp)
target_link_libraries(myapp PRIVATE fmt::fmt nlohmann_json::nlohmann_json)
```

---

### Ví dụ 10: Modern CMake với Presets (CMake 3.19+)

**CMakePresets.json:**
```json
{
    "version": 3,
    "configurePresets": [
        {
            "name": "default",
            "binaryDir": "${sourceDir}/build/default",
            "cacheVariables": {
                "CMAKE_BUILD_TYPE": "Release",
                "CMAKE_CXX_STANDARD": "17"
            }
        },
        {
            "name": "debug",
            "inherits": "default",
            "binaryDir": "${sourceDir}/build/debug",
            "cacheVariables": {
                "CMAKE_BUILD_TYPE": "Debug",
                "BUILD_TESTS": "ON"
            }
        }
    ],
    "buildPresets": [
        { "name": "default", "configurePreset": "default" },
        { "name": "debug", "configurePreset": "debug" }
    ]
}
```

**Sử dụng:**
```bash
cmake --preset debug
cmake --build --preset debug
```

---

## 10. Best Practices

### ✅ NÊN làm

1. **Luôn dùng out-of-source build**
   ```bash
   cmake -S . -B build      # ✓ Đúng
   cmake .                  # ✗ Sai (build trong source)
   ```

2. **Liệt kê file thủ công, KHÔNG dùng GLOB**
   ```cmake
   # ✓ Đúng
   add_executable(app main.cpp utils.cpp)
   
   # ✗ Sai (CMake không tự detect file mới)
   file(GLOB SRCS "src/*.cpp")
   add_executable(app ${SRCS})
   ```

3. **Dùng target-based commands**
   ```cmake
   # ✓ Modern CMake
   target_include_directories(app PRIVATE include/)
   target_link_libraries(app PRIVATE mylib)
   
   # ✗ Old CMake (avoid)
   include_directories(include/)
   link_libraries(mylib)
   ```

4. **Chỉ định scope rõ ràng (PUBLIC/PRIVATE/INTERFACE)**

5. **Đặt `CMAKE_CXX_STANDARD_REQUIRED ON`**

6. **Dùng version CMake mới (>= 3.15)**

### ❌ KHÔNG nên

1. ❌ Set compiler thẳng (`CMAKE_CXX_COMPILER`) trong `CMakeLists.txt`
2. ❌ Hardcode đường dẫn tuyệt đối
3. ❌ Dùng `link_directories()` (đã deprecated)
4. ❌ Build trong source directory
5. ❌ Bỏ qua `cmake_minimum_required`

---

## 11. Troubleshooting

### Vấn đề thường gặp

#### 1. "CMake Error: Could not find package XXX"
```bash
# Cài thư viện thiếu
sudo apt install libssl-dev    # Linux
brew install openssl           # macOS

# Hoặc chỉ định path
cmake -DOpenSSL_ROOT=/path/to/openssl ..
```

#### 2. Cache cũ gây lỗi
```bash
# Xóa cache build hoàn toàn
rm -rf build/
cmake -S . -B build
```

#### 3. Build chậm
```bash
# Dùng Ninja + parallel
cmake -G Ninja -S . -B build
cmake --build build -j$(nproc)
```

#### 4. Không thấy compile command
```cmake
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
```

#### 5. Debug CMake script
```cmake
message(STATUS "Variable value: ${MY_VAR}")
message(STATUS "All sources: ${SOURCES}")

# In tất cả biến
get_cmake_property(_vars VARIABLES)
foreach(_var ${_vars})
    message(STATUS "${_var} = ${${_var}}")
endforeach()
```

---

## Tổng Kết - Checklist Cho Project Mới

```cmake
# ✅ Template chuẩn cho project mới
cmake_minimum_required(VERSION 3.15)              # 1. Version
project(MyApp VERSION 1.0 LANGUAGES CXX)          # 2. Project info

set(CMAKE_CXX_STANDARD 17)                        # 3. C++ standard
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)             # Cho IDE

if(NOT CMAKE_BUILD_TYPE)                          # 4. Default build type
    set(CMAKE_BUILD_TYPE Release)
endif()

# 5. Tạo target
add_executable(myapp src/main.cpp)

# 6. Properties
target_include_directories(myapp PRIVATE include/)
target_compile_features(myapp PRIVATE cxx_std_17)
target_compile_options(myapp PRIVATE -Wall -Wextra)
```

**Lệnh build chuẩn:**
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
./build/myapp
```

---

## Tài Liệu Tham Khảo

- **Tài liệu chính thức:** https://cmake.org/cmake/help/latest/
- **Modern CMake guide:** https://cliutils.gitlab.io/modern-cmake/
- **Awesome CMake:** https://github.com/onqtam/awesome-cmake

---

> **Học tốt CMake = Đọc + Thực hành.** Hãy bắt đầu từ Ví dụ 1 và build từng cấp độ. Sau khi hoàn thành Ví dụ 8, bạn có thể tự tin viết CMake chuyên nghiệp cho mọi project C/C++!
