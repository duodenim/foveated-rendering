cmake_minimum_required(VERSION 3.5)
project(ENGINE CXX)

set(CMAKE_CXX_STANDARD 14)

set(ENGINE_ROOT ${PROJECT_SOURCE_DIR})

if (WIN32)
  set(BUILD_SHARED_LIBS FALSE)
endif()

add_subdirectory(deps/glad)
add_subdirectory(deps/SDL2-2.0.7)
add_subdirectory(deps/freetype-2.8.1)
add_subdirectory(deps/SDL_mixer-cmake-2.0.1-cmake)
add_subdirectory(deps/assimp-4.1.0)
add_subdirectory(deps/imgui)

add_subdirectory(src/Engine/Renderer)

set(ENG_SOURCE_FILES src/Engine/EngineCore.cpp
                 src/Engine/EngineCore.h
                 src/Engine/GameObject.cpp
                 src/Engine/GameObject.h
                 src/Engine/InputManager.cpp
                 src/Engine/InputManager.h
                 src/Engine/Component.cpp
                 src/Engine/Component.h
                 src/Engine/AudioSystem.cpp
                 src/Engine/AudioSystem.h
                 src/Engine/Config.cpp
                 src/Engine/Config.h
                 src/Engine/FileLoader.cpp
                 src/Engine/FileLoader.h
                 src/Engine/CommonTypes.h
                 src/Engine/Components/MeshComponent.cpp
                 src/Engine/Components/MeshComponent.h
                 src/Engine/Components/InputComponent.cpp
                 src/Engine/Components/InputComponent.h
                 src/Engine/Components/AudioSourceComponent.cpp
                 src/Engine/Components/AudioSourceComponent.h
                 src/Engine/Components/CameraComponent.cpp
                 src/Engine/Components/CameraComponent.h
                 src/Engine/Components/CollisionComponent.cpp
                 src/Engine/Components/CollisionComponent.h
                 src/Engine/Components/CollisionBox2DComponent.cpp
                 src/Engine/Components/CollisionBox2DComponent.h
                 src/Engine/Components/CollisionCircle2DComponent.cpp
                 src/Engine/Components/CollisionCircle2DComponent.h
                 src/Engine/Controller.cpp
                 src/Engine/Controller.h
                 src/Engine/Log.cpp
                 src/Engine/Log.h
                 src/Engine/string_ops.h
                 src/Engine/string_ops.cpp
                 src/Engine/CommandArgs.cpp
                 src/Engine/Map.cpp
                 src/Engine/Components/DirectionalLightComponent.cpp
                 src/Engine/Components/BillboardComponent.cpp
                 src/Engine/Components/SpriteComponent.cpp
                 src/Engine/Components/TextComponent.cpp
                 src/Engine/Transform.cpp)

include_directories(deps/assimp-4.1.0/include)
include_directories(${CMAKE_BINARY_DIR}/deps/game-engine/deps/assimp-4.1.0/include)
include_directories(deps/glm/glm)
include_directories(deps/SDL2-2.0.7/include)
include_directories(deps/freetype-2.8.1/include)
include_directories(deps/SDL_mixer-cmake-2.0.1-cmake)
include_directories(deps/json)

add_library(ENGINE STATIC ${ENG_SOURCE_FILES})

if(NOT WIN32)
  set(GLAD_LIBS glad45 dl)
  set(FS_LIBS stdc++fs)
else()
  set(GLAD_LIBS glad45)
  set(FS_LIBS)
endif()

target_link_libraries(ENGINE ENGINE_RENDERER assimp SDL2 SDL_mixer ${GLAD_LIBS} ${FS_LIBS} freetype IMGUI)
