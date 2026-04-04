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

option(BUILD_APP "Build the main application" ON)
if(BUILD_APP)
    add_executable(wriscv_app "${SRC_DIR}/main.cpp")
    target_link_libraries(wriscv_app PRIVATE wriscv_core)
endif()

option(BUILD_TESTING "Build tests" ON)
if(BUILD_TESTING)
    enable_testing()
    
    include(FetchContent)
    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git 
        GIT_TAG        v3.4.0
    )
    FetchContent_MakeAvailable(Catch2)
    
    list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
    include(Catch)
    
    add_executable(wriscv_test_rv32i "${TEST_DIR}/test_rv32i.cpp")
    target_link_libraries(wriscv_test_rv32i PRIVATE wriscv_core Catch2::Catch2WithMain)
    catch_discover_tests(wriscv_test_rv32i)

        add_executable(wriscv_test_rv32m "${TEST_DIR}/test_rv32m.cpp")
    target_link_libraries(wriscv_test_rv32m PRIVATE wriscv_core Catch2::Catch2WithMain)
    catch_discover_tests(wriscv_test_rv32m)
endif()