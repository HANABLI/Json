# CMakeLists.txt for Json Module
#
# © 2024 by Hatem Nabli

cmake_minimum_required(VERSION 3.8)
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

target_include_directories(${this} PUBLIC includes)
target_link_libraries(${this} PUBLIC 

)

add_subdirectory(test)