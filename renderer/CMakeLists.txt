cmake_minimum_required(VERSION 3.5)
project(DX CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_subdirectory(${CMAKE_SOURCE_DIR}/deps/game-engine)

set(SOURCE_FILES src/main.cpp
                 src/World/Player.cpp
                 src/UI/WindowComponent.cpp)

include_directories(deps/game-engine/src/Engine)
include_directories(deps/game-engine/deps/glm/glm)
include_directories(deps/game-engine/deps/SDL2-2.0.7/include)
include_directories(deps/game-engine/deps/glad/include)
include_directories(deps/game-engine/deps/json)
include_directories(deps/game-engine/deps/SDL_mixer-cmake-2.0.1-cmake)
include_directories(deps/game-engine/deps/imgui)

add_executable(dx ${SOURCE_FILES})
target_link_libraries(dx ENGINE)
