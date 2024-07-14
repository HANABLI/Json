# CMakeLists.txt for Json tests Module
#
# © 2024 by Hatem Nabli

cmake_minimum_required(VERSION 3.8)
set(this JsonTests)

set(Sources 
    src/JsonTests.cpp
)

add_executable(${this} ${Sources})
set_target_properties(${this} PROPERTIES
    FOLDER Tests
)

target_include_directories(${this} PRIVATE ..)

target_link_libraries(${this} PUBLIC
    gtest_main
    Json
)

add_test(
    NAME ${this}
    COMMAND ${this}
)