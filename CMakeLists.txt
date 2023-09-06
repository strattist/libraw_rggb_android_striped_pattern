cmake_minimum_required(VERSION 3.16)

project(libraw_stripes CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

### Dependencies

# CLI11
FetchContent_Declare(
    CLI11
    GIT_REPOSITORY https://github.com/CLIUtils/CLI11
    GIT_TAG v2.3.2
)
FetchContent_MakeAvailable(CLI11)

# spdlog
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG        v1.11.0
)
FetchContent_MakeAvailable(spdlog)

# LibRAW
FetchContent_Declare(
    libraw
    GIT_REPOSITORY https://github.com/LibRaw/LibRaw.git
    GIT_TAG        0.21.1
)
FetchContent_MakeAvailable(libraw)
set(LIBRAW_PATH ${libraw_SOURCE_DIR} CACHE PATH "Path to libraw directory")
set(ENABLE_OPENMP OFF CACHE BOOL "Disable openmp for libraw")
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Disabling shared libs")
FetchContent_Declare(
    libraw-cmake
    GIT_REPOSITORY https://github.com/LibRaw/LibRaw-cmake.git
)
FetchContent_MakeAvailable(libraw-cmake)

# OpenCV
find_package(OpenCV COMPONENTS core imgcodecs REQUIRED)

### Application

add_executable(libraw_stripes main.cpp)
target_link_libraries(libraw_stripes PRIVATE
    CLI11::CLI11
    libraw::libraw
    opencv_core
    opencv_imgcodecs
    spdlog::spdlog
)


