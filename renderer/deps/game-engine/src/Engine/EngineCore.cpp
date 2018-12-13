#include "EngineCore.h"
#include "Config.h"
#include <SDL.h>
#include "Renderer/Frontend.h"
#include <algorithm>

EngineCore::EngineCore() {
  state = EngineState::GAME_STARTUP;
  SDL_Startup();
  RenderFrontend::Init();
  Input::Init();
  Audio::Init();
  lastFrameTime = 0;
  currentFrameTime = 0;
  mMap = SpawnObject<Map>();
}

EngineCore::~EngineCore() {
  DestroyAllObjects();
  Input::Shutdown();
  Audio::Shutdown();
  RenderFrontend::Shutdown();
  SDL_Shutdown();
}

void EngineCore::BeginGame() {
  state = EngineState ::GAME_RUNNING;
  while (state == EngineState::GAME_RUNNING) {

    lastFrameTime = currentFrameTime;

    currentFrameTime = SDL_GetPerformanceCounter();

    deltaTime = currentFrameTime - lastFrameTime;

    //Run events
    deltaTime /= SDL_GetPerformanceFrequency();
    if(deltaTime > 1){
      deltaTime = 0.0f;
    }
    Input::PollInputs();
    if(Input::GetClose()){
      Quit();
    }

    RenderFrontend::BeginFrame();
    Update(deltaTime);
    RenderFrontend::EndFrame();

  }
}
void EngineCore::Quit() {
  state = EngineState::GAME_SHUTDOWN;
}

void EngineCore::SDL_Startup() {
  SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER);
}

void EngineCore::SDL_Shutdown() {
  SDL_Quit();
}

void EngineCore::DestroyObject(GameObject* object) {
  mDestroyQueue.push_back(object);
}

void EngineCore::DestroyAllObjects() {
  GameObject *temp;
  while(!mObjects.empty()) {
    temp = mObjects.back();
    mObjects.pop_back();
    delete temp;
  }
}

void EngineCore::Update(float deltaTime) {
  ClearDestroyQueue();
  RunCollision();
  for (int i = 0; i < mObjects.size(); i++) {
    mObjects[i]->Update(deltaTime);
  }
}

void EngineCore::ClearDestroyQueue() {
  GameObject *temp;
  while(!mDestroyQueue.empty()) {
    temp = mDestroyQueue.back();
    mDestroyQueue.pop_back();
    Remove(temp);
  }
}
void EngineCore::Remove(GameObject* object) {
  auto it = std::find(mObjects.begin(), mObjects.end(), object);
  if (it != mObjects.end()) {
    mObjects.erase(it);
    mObjects.shrink_to_fit();
    delete object;
  }

}

void EngineCore::RegisterCollider(CollisionComponent* collider) {
  mColliders.push_back(collider);
}

void EngineCore::RunCollision() {
  if (mColliders.size() < 2) {
    return;
  }
  for(int i = 0; i < mColliders.size() - 1; i++) {
    for(int k = i + 1; k < mColliders.size(); k++) {
      if(mColliders[i]->CheckCollision(mColliders[k])) {
        if(mColliders[i]->owner != NULL && mColliders[k]->owner != NULL) {
          mColliders[i]->owner->Collision(mColliders[k]->owner);
          mColliders[k]->owner->Collision(mColliders[i]->owner);
        }
      }
    }
  }
}

void EngineCore::RemoveCollider(CollisionComponent *collider) {
  auto it = std::find(mColliders.begin(), mColliders.end(), collider);
  mColliders.erase(it);
  mColliders.shrink_to_fit();
}

//TODO - Try to remove singleton design from engine
EngineCore *EngineCore::engine = 0;
EngineCore *EngineCore::GetEngine() {
  if(!engine)
    engine = new EngineCore();
  return engine;
}
Map *EngineCore::GetMap() const {
  return mMap;
}
