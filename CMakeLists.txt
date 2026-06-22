cmake_minimum_required(VERSION 3.16)
project(orderbook LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

if(MSVC)
    add_compile_options(/W4 /permissive-)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

add_library(orderbook_lib
    src/order_book.cpp
    src/parser.cpp
    src/metrics.cpp
    src/visualizer.cpp
)
target_include_directories(orderbook_lib PUBLIC include)

add_executable(orderbook_demo src/main.cpp)
target_link_libraries(orderbook_demo PRIVATE orderbook_lib)

enable_testing()
add_executable(orderbook_tests tests/test_main.cpp)
target_link_libraries(orderbook_tests PRIVATE orderbook_lib)
add_test(NAME unit COMMAND orderbook_tests)

add_executable(orderbook_bench benchmarks/throughput.cpp)
target_link_libraries(orderbook_bench PRIVATE orderbook_lib)
