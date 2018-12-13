cmake_minimum_required(VERSION 3.5)
project(GLAD4_5 C)

set(CMAKE_C_STANDARD 99)
include_directories(include)

add_library(glad45 STATIC src/glad.c)