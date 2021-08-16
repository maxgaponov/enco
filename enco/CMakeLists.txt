cmake_minimum_required (VERSION 3.5)

add_executable(${PROJECT_NAME} main.cpp syscall_manager.cpp)
message(info, ${PROJECT_SOURCE_DIR})
target_include_directories(${PROJECT_NAME} PUBLIC
    ${PROJECT_SOURCE_DIR}/enco/include
    ${lua_SOURCE_DIR}
    ${sol_SOURCE_DIR}
)

target_link_libraries(${PROJECT_NAME} lua)
