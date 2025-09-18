cmake_minimum_required(VERSION 3.10)

project(marconautas)

add_executable(marconautas
    src/main.cpp

    deps/imgui/imgui.cpp
    deps/imgui/imgui_widgets.cpp
    deps/imgui/imgui_tables.cpp
    deps/imgui/imgui_draw.cpp
    deps/imgui/imgui_demo.cpp
    deps/imgui/backends/imgui_impl_opengl3.cpp
    deps/imgui/backends/imgui_impl_glfw.cpp
)

target_include_directories(marconautas
    PRIVATE
    ${CMAKE_SOURCE_DIR}/deps/imgui/
)

target_link_libraries(marconautas
    PRIVATE
    paho-mqtt3c
    glfw3
    X11
)

if(WIN32)
    target_link_libraries(marconautas PRIVATE opengl32)
else()
    target_link_libraries(marconautas PRIVATE GL)
endif()