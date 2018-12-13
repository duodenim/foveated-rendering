cmake_minimum_required(VERSION 3.7)
project(VK_RENDERER CXX)

find_package(Vulkan)

if (Vulkan_FOUND)
else()
  message( FATAL_ERROR "Vulkan Libraries Not Found")
endif()


set(CMAKE_CXX_STANDARD 17)
set(VK_RENDERER_SRC VKRenderer.cpp VKError.cpp VKDevice.cpp VKSurface.cpp VKImage.cpp VKBuffer.cpp imgui_impl_vulkan.cpp VKFrameBuffer.cpp GazePoint.cpp)

include_directories(${ENGINE_ROOT}/deps/glm/glm)
include_directories(${ENGINE_ROOT}/deps/SDL2-2.0.7/include)
include_directories(${ENGINE_ROOT}/deps/VulkanMemoryAllocator/src)
include_directories(${Vulkan_INCLUDE_DIRS})
include_directories(${ENGINE_ROOT}/deps/imgui)

add_library(ENGINE_RENDERER_VK STATIC ${VK_RENDERER_SRC})

target_link_libraries(ENGINE_RENDERER_VK ${Vulkan_LIBRARIES})
