cmake_minimum_required(VERSION 3.31)
project(mas_gc C)

set(CMAKE_C_STANDARD 11)

add_library(munit STATIC
    munit/munit.c
)
target_include_directories(munit PUBLIC munit)

add_executable(mas_gc tester.c object.c stack.c vm.c)
target_link_libraries(mas_gc PRIVATE munit)
