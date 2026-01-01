# ============================================================================
# Trading Tool - High-Performance C++ Backend with Python Bindings
# Root CMakeLists.txt
# ============================================================================
cmake_minimum_required(VERSION 3.18)
project(TradingTool VERSION 0.0.1 LANGUAGES CXX)

# ============================================================================
# Build Configuration
# ============================================================================
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")

# ============================================================================
# Compiler Flags - Performance Optimizations
# ============================================================================
if(CMAKE_BUILD_TYPE MATCHES Release)
    set(OPTIMIZATION_FLAGS
        -O3                     # Maximum optimization
        -march=native           # CPU-specific optimizations
        -mtune=native
        -flto                   # Link-time optimization
        -ffast-math            # Aggressive math optimizations
        -funroll-loops         # Loop unrolling
        -fomit-frame-pointer   # Remove frame pointer for extra register
    )
    
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        list(APPEND OPTIMIZATION_FLAGS
            -ftree-vectorize
            -fopt-info-vec-optimized
        )
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        list(APPEND OPTIMIZATION_FLAGS
            -Rpass=loop-vectorize
        )
    endif()
    
elseif(CMAKE_BUILD_TYPE MATCHES Debug)
    set(DEBUG_FLAGS
        -g
        -O0
        -fsanitize=address
        -fsanitize=undefined
        -fno-omit-frame-pointer
    )
endif()

# Warning flags (always enabled)
set(WARNING_FLAGS
    -Wall
    -Wextra
    -Wpedantic
    -Wshadow
    -Wconversion
    -Wnon-virtual-dtor
)

# Make flags available globally
add_compile_options(${WARNING_FLAGS})
if(CMAKE_BUILD_TYPE MATCHES Release)
    add_compile_options(${OPTIMIZATION_FLAGS})
elseif(CMAKE_BUILD_TYPE MATCHES Debug)
    add_compile_options(${DEBUG_FLAGS})
    add_link_options(-fsanitize=address -fsanitize=undefined)
endif()

# ============================================================================
# Dependencies
# ============================================================================

# Find Python
find_package(Python COMPONENTS Interpreter Development REQUIRED)
message(STATUS "Python: ${Python_VERSION}")
message(STATUS "Python Include: ${Python_INCLUDE_DIRS}")

# Find pybind11
find_package(pybind11 CONFIG REQUIRED)
message(STATUS "pybind11: ${pybind11_VERSION}")

# Threading (required for ThreadPool)
find_package(Threads REQUIRED)

# Optional: Benchmark library
find_package(benchmark QUIET)
if(benchmark_FOUND)
    message(STATUS "Google Benchmark found - building benchmarks")
    set(BUILD_BENCHMARKS ON)
else()
    message(STATUS "Google Benchmark not found - skipping benchmarks")
    set(BUILD_BENCHMARKS OFF)
endif()

# Optional: GoogleTest
find_package(GTest QUIET)
if(GTest_FOUND)
    message(STATUS "Google Test found - building C++ tests")
    set(BUILD_CPP_TESTS ON)
    enable_testing()
else()
    message(STATUS "Google Test not found - skipping C++ tests")
    set(BUILD_CPP_TESTS OFF)
endif()

# ============================================================================
# Global Include Directories
# ============================================================================
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/backtest
    ${CMAKE_SOURCE_DIR}/include/core
    ${CMAKE_SOURCE_DIR}/include/utils
    ${CMAKE_SOURCE_DIR}/include/strategies
    ${CMAKE_SOURCE_DIR}/include/execution
    ${CMAKE_SOURCE_DIR}/include/storage
)

# ============================================================================
# Add Subdirectories (Modular Build)
# ============================================================================

# Build C++ libraries first
add_subdirectory(src/core)
add_subdirectory(src/utils)

# Future subdirectories (uncomment when ready)
# add_subdirectory(src/strategies)
# add_subdirectory(src/execution)
# add_subdirectory(src/storage)
add_subdirectory(src/backtest)

# Build Python bindings (depends on libraries above)
add_subdirectory(src)

# Optional: Tests and benchmarks
if(BUILD_CPP_TESTS)
    add_subdirectory(tests/cpp)
endif()

if(BUILD_BENCHMARKS)
    add_subdirectory(benchmarks)
endif()

# ============================================================================
# Installation Rules
# ============================================================================
install(DIRECTORY include/
    DESTINATION include
    FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp"
)

# ============================================================================
# Summary
# ============================================================================
message(STATUS "")
message(STATUS "========================================")
message(STATUS "Trading Tool Build Configuration")
message(STATUS "========================================")
message(STATUS "Build Type:        ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard:      C++${CMAKE_CXX_STANDARD}")
message(STATUS "Install Prefix:    ${CMAKE_INSTALL_PREFIX}")
message(STATUS "Python Module:     trading_core")
message(STATUS "C++ Tests:         ${BUILD_CPP_TESTS}")
message(STATUS "Benchmarks:        ${BUILD_BENCHMARKS}")
message(STATUS "========================================")
message(STATUS "")