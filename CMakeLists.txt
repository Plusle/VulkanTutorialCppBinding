cmake_minimum_required(VERSION 3.15.0)

set(VCPKG_TOOLCHAIN_FILE "$ENV{VCPKG_TOOLCHAIN_FILE_PATH}/vcpkg.cmake")
message("Toolchain file is detected: ${VCPKG_TOOLCHAIN_FILE}")
set(CMAKE_TOOLCHAIN_FILE ${VCPKG_TOOLCHAIN_FILE} CACHE STRING "Vcpkg toolchain file")

project(TestVulkan)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)


find_package(Vulkan REQUIRED)
include_directories(${Vulkan_INCLUDE_DIRS})

find_package(glfw3 CONFIG REQUIRED)

include_directories(include)
add_subdirectory(src)