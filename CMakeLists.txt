cmake_minimum_required(VERSION 3.14)

project(spaceinvaders)

set(CMAKE_CXX_STANDARD 17)

add_executable(spaceinvaders src/System/memory.cpp src/System/invaders.cpp 
                src/System/main.cpp src/8080/cpu.cpp)

target_compile_options(spaceinvaders PRIVATE -Wall)