cmake_minimum_required(VERSION 3.20)
project(lixxuid_cs2 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(FATAL_ERROR "CS2 is strictly x64. Please build for 64-bit architecture.")
endif()

# Output configuration
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Project include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/deps
    ${CMAKE_CURRENT_SOURCE_DIR}/deps/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/deps/minhook/include
)

# Source files (excluding injector)
file(GLOB_RECURSE SRC_FILES
    "src/*.cpp"
    "src/*.hpp"
    "src/*.h"
)

# Vendored MinHook source files
file(GLOB MINHOOK_SRC
    "deps/minhook/src/*.c"
    "deps/minhook/src/hde/*.c"
)

# Vendored ImGui source files
file(GLOB IMGUI_SRC
    "deps/imgui/*.cpp"
)

# Main CS2 Internal DLL
add_library(${PROJECT_NAME} SHARED
    ${SRC_FILES}
    ${MINHOOK_SRC}
    ${IMGUI_SRC}
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    WIN32_LEAN_AND_MEAN
    NOMINMAX
    _CRT_SECURE_NO_WARNINGS
    UNICODE
    _UNICODE
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    d3d11
    dxgi
    user32
    gdi32
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_EXTENSIONS OFF
)
