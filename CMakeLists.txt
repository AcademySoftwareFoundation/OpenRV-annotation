cmake_minimum_required(VERSION 3.19)
project(TwkPaint VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(ENABLE_COVERAGE "Build with code-coverage instrumentation" OFF)
option(ENABLE_SANITIZERS "Build with AddressSanitizer and UBSanitizer" OFF)

if(ENABLE_COVERAGE)
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
        add_compile_options(--coverage -O0 -g)
        add_link_options(--coverage)
    else()
        message(WARNING "ENABLE_COVERAGE is only supported with Clang or GCC")
    endif()
endif()

if(ENABLE_SANITIZERS)
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
        add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer -g)
        add_link_options(-fsanitize=address,undefined)
    else()
        message(WARNING "ENABLE_SANITIZERS is only supported with Clang or GCC")
    endif()
endif()

# Imath provides the half-precision float type used by TwkMath.
# Force static linking — Imath's macro defaults to SHARED which breaks WASM
# and other platforms that don't support dynamic linking.
include(FetchContent)
FetchContent_Declare(
    Imath
    GIT_REPOSITORY https://github.com/AcademySoftwareFoundation/Imath.git
    GIT_TAG        v3.1.9
    GIT_SHALLOW    TRUE
)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(Imath)

add_subdirectory(TwkMath)
add_subdirectory(TwkPaint)

# Tests are native-only — skip when cross-compiling (e.g. Emscripten/WASM).
if(NOT CMAKE_CROSSCOMPILING)
    enable_testing()
    add_subdirectory(tests)
endif()
