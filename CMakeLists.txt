cmake_minimum_required(VERSION 3.14...3.25)

project(SmartSensorAnomaly
    VERSION 1.0.0
    DESCRIPTION "Edge AI Anomaly Detection Pipeline for Smart Sensors"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

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

include_directories(include)

file(GLOB_RECURSE SRC_FILES
    "src/*.cpp"
)

# MicroPrintf and DebugLog are not included in the pre-built libtensorflow-microlite.a
set(TFLM_SRC ${CMAKE_BINARY_DIR}/_deps/tflite_micro-src)
list(APPEND SRC_FILES
    ${TFLM_SRC}/tensorflow/lite/micro/micro_log.cc
    ${TFLM_SRC}/tensorflow/lite/micro/debug_log.cc
)

add_executable(${PROJECT_NAME} ${SRC_FILES})

# Compiler flags AFTER add_executable
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(${PROJECT_NAME}
        PRIVATE
            -Wall
            -Wextra
            -Wshadow
            -DTF_LITE_STATIC_MEMORY
    )
elseif(MSVC)
    target_compile_options(${PROJECT_NAME}
        PRIVATE
            /W4
    )
endif()

target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${CMAKE_BINARY_DIR}/_deps/tflite_micro-src
        ${CMAKE_BINARY_DIR}/_deps/tflite_micro-src/tensorflow/lite/micro/tools/make/downloads/flatbuffers/include
        ${CMAKE_BINARY_DIR}/_deps/tflite_micro-src/tensorflow/lite/micro/tools/make/downloads/gemmlowp
        ${CMAKE_BINARY_DIR}/_deps/tflite_micro-src/tensorflow/lite/micro/tools/make/downloads/ruy
        ${CMAKE_BINARY_DIR}/_deps/tflite_micro-src/third_party/flatbuffers/include
        ${CMAKE_BINARY_DIR}/_deps/tflite_micro-src/third_party/gemmlowp
        ${CMAKE_BINARY_DIR}/_deps/tflite_micro-src/third_party/ruy
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        /home/vijayrnair/PProjects/smart_sensor_anomaly/build/_deps/tflite_micro-src/gen/linux_x86_64_release_gcc/lib/libtensorflow-microlite.a
)

message(STATUS "Configured ${PROJECT_NAME} successfully using ${CMAKE_CXX_COMPILER_ID}")
