cmake_minimum_required(VERSION 3.10)
project(wriscv CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(EXT_DIR "${SRC_DIR}/extensions")
set(TEST_DIR "${CMAKE_CURRENT_SOURCE_DIR}/test")

add_library(wriscv_core STATIC)

target_sources(wriscv_core PRIVATE
    "${SRC_DIR}/ram.cpp"
)

target_include_directories(wriscv_core PUBLIC
    "${SRC_DIR}"
    "${EXT_DIR}"
)

add_executable(wriscv "${SRC_DIR}/main.cpp")
target_link_libraries(wriscv PRIVATE wriscv_core)