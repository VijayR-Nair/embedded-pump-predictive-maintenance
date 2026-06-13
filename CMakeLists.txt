cmake_minimum_required(VERSION 3.14...3.25)

# 1. Define the project name, version, and language
project(SmartSensorAnomaly
    VERSION 1.0.0
    DESCRIPTION "Edge AI Anomaly Detection Pipeline for Smart Sensors"
    LANGUAGES CXX
)

# 2. Enforce Modern C++ Standard (C++17)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF) # Disables compiler-specific extensions to ensure portability

# 3. Define compiler flags (Strict warnings + Treat warnings as errors)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(
        -Wall          # Enable all standard warnings
        -Wextra        # Enable extra useful warnings
        -Wpedantic     # Enforce strict ISO C++ compliance
        -Werror        # Treat ALL warnings as errors (forces you to write clean code)
        -Wshadow       # Warn if a local variable shadows another variable
        -Wconversion   # Warn if implicit type conversions might lose data
    )
elseif(MSVC)
    # If you happen to build on Windows using MSVC compiler
    add_compile_options(/W4 /WX)
endif()

# Fetch external dependencies
include(FetchContent)

FetchContent_Declare(
    tflite_micro
    GIT_REPOSITORY https://github.com/tensorflow/tflite-micro.git
    GIT_TAG main
    SOURCE_DIR ${CMAKE_BINARY_DIR}/_deps/tflite_micro-src
    BINARY_DIR ${CMAKE_BINARY_DIR}/_deps/tflite_micro-build
)

FetchContent_MakeAvailable(tflite_micro)

# 4. Include directories (Tells CMake where your header files live)
# This mimics the "include/smart_sensor/" structure we planned
include_directories(include)

# 5. Collect your source code files
# For now, we assume you have main.cpp and a virtual sensor driver
file(GLOB_RECURSE SRC_FILES 
    "src/*.cpp"
)

# 6. Define the final executable binary
add_executable(${PROJECT_NAME} ${SRC_FILES})

target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${CMAKE_BINARY_DIR}/_deps/tflite_micro-src
        ${CMAKE_BINARY_DIR}/_deps/tflite_micro-src/third_party/flatbuffers/include
        ${CMAKE_BINARY_DIR}/_deps/tflite_micro-src/third_party/gemmlowp
        ${CMAKE_BINARY_DIR}/_deps/tflite_micro-src/third_party/ruy
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        tensorflow-lite-micro
)

# 7. Print a helpful message when configuration finishes
message(STATUS "Configured ${PROJECT_NAME} successfully using ${CMAKE_CXX_COMPILER_ID}")
