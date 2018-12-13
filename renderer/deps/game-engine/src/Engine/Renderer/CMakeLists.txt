cmake_minimum_required(VERSION 3.5)
project(ENGINE_RENDERER CXX)

add_subdirectory(Backends/Vulkan)

set(CMAKE_CXX_STANDARD 17)
set(RENDERER_SRC Frontend.cpp)

include_directories(${ENGINE_ROOT}/deps/glm/glm)
include_directories(${ENGINE_ROOT}/deps/SDL2-2.0.7/include)
include_directories(${ENGINE_ROOT}/deps/assimp-4.1.0/include)
include_directories(${CMAKE_BINARY_DIR}/deps/game-engine/deps/assimp-4.1.0/include)
include_directories(${ENGINE_ROOT}/deps/freetype-2.8.1/include)
include_directories(${Vulkan_INCLUDE_DIR})
include_directories(${ENGINE_ROOT}/deps/VulkanMemoryAllocator/src)
include_directories(${ENGINE_ROOT}/deps/imgui)

add_library(ENGINE_RENDERER STATIC ${RENDERER_SRC})

target_link_libraries(ENGINE_RENDERER ENGINE_RENDERER_VK)