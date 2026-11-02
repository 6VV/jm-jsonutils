cmake_minimum_required(VERSION 3.20)
project(jm_jsonutils LANGUAGES CXX)

add_library(jm_jsonutils INTERFACE)
add_library(jm::jsonutils ALIAS jm_jsonutils)
target_include_directories(jm_jsonutils INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/third_party>)
target_compile_features(jm_jsonutils INTERFACE cxx_std_14)

if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    # /permissive-：宏在消费者 TU 展开所需的标准一致性；
    # /utf-8：头文件与源文件统一 UTF-8（含中文注释），避免 MSVC 本地代码页误读。
    target_compile_options(jm_jsonutils INTERFACE /permissive- /utf-8)
    # 警告选项只挂测试/示例 target，不外溢给库消费者。
    set(JM_TEST_COMPILE_OPTIONS "")
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(JM_TEST_COMPILE_OPTIONS -Wall -Wextra)
else()
    set(JM_TEST_COMPILE_OPTIONS "")
endif()

enable_testing()

function(jm_add_test name)
    add_executable(${name} tests/${name}.cpp ${ARGN})
    target_link_libraries(${name} PRIVATE jm::jsonutils)
    if(JM_TEST_COMPILE_OPTIONS)
        target_compile_options(${name} PRIVATE ${JM_TEST_COMPILE_OPTIONS})
    endif()
    add_test(NAME ${name} COMMAND ${name})
endfunction()

jm_add_test(test_smoke)
