# CMakeLists.txt for Json Module
#
# © 2024 by Hatem Nabli

cmake_minimum_required(VERSION 3.20)
set(this Json)


set(Headers 
    include/Json/Json.hpp
)

set(Sources
    src/Json.cpp
)

add_library(${this} STATIC ${Sources} ${Headers})
set_target_properties(${this} PROPERTIES 
    FOLDER Libraries
)

target_include_directories(${this} PUBLIC include)
target_link_libraries(${this} PUBLIC 
    StringUtils
    Utf8
)

add_subdirectory(test)