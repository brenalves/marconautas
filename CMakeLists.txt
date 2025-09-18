cmake_minimum_required(VERSION 3.10)

project(marconautas)

add_executable(marconautas
    src/main.cpp
)

target_link_libraries(marconautas
    PRIVATE
    paho-mqtt3c
)