cmake_minimum_required(VERSION 3.15)

project(marconautas)

add_subdirectory(deps/glfw)
add_subdirectory(deps/paho.mqtt.c)

add_executable(marconautas
    src/main.cpp
    src/app.h src/app.cpp
    src/window.h src/window.cpp
    src/client.h src/client.cpp

    deps/imgui/imgui.cpp
    deps/imgui/imgui_widgets.cpp
    deps/imgui/imgui_tables.cpp
    deps/imgui/imgui_draw.cpp
    deps/imgui/imgui_demo.cpp
    deps/imgui/backends/imgui_impl_opengl3.cpp
    deps/imgui/backends/imgui_impl_glfw.cpp
    deps/stb_image/src/stb_image.cpp
)

target_include_directories(marconautas
    PRIVATE
    ${CMAKE_SOURCE_DIR}/deps/imgui/
    ${CMAKE_SOURCE_DIR}/deps/stb_image/include
)

target_link_libraries(marconautas
    PRIVATE
    paho-mqtt3c
    glfw
)

add_custom_command(
    TARGET marconautas POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/images $<TARGET_FILE_DIR:marconautas>/images
)

if(WIN32)
    target_link_libraries(marconautas PRIVATE opengl32)

    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
else()
    target_link_libraries(marconautas PRIVATE GL X11)
endif()