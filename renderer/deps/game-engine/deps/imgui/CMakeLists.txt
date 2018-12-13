cmake_minimum_required(VERSION 3.5)
project(IMGUI CXX)

set(CMAKE_CXX_STANDARD 14)

add_library(IMGUI STATIC imgui.cpp imgui_draw.cpp imgui_widgets.cpp imgui_demo.cpp)
