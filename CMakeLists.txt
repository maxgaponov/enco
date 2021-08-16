cmake_minimum_required (VERSION 3.5)

project(enco)

set(CMAKE_CXX_STANDARD 17)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

add_subdirectory(third_party/lua)
add_subdirectory(third_party/sol3)

add_subdirectory(enco)
