cmake_minimum_required(VERSION 3.10)

project(marconautas)

add_subdirectory(dependencies/glfw)
add_subdirectory(dependencies/imgui)

add_executable(marconautas
    src/main.cpp
)

target_link_libraries(marconautas PRIVATE glfw GL imgui)
target_include_directories(marconautas PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/imgui/backends)