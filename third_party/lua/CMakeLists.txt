cmake_minimum_required (VERSION 3.5)

project(lua)

add_library(${PROJECT_NAME} STATIC onelua.c)
target_compile_definitions(${PROJECT_NAME} PRIVATE MAKE_LIB LUA_USE_POSIX)
