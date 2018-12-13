//Contains State Information for the currently running game

#ifndef ENGINECORE_H
#define ENGINECORE_H


#include "InputManager.h"
#include "AudioSystem.h"
#include "CommonTypes.h"
#include "Components/CollisionComponent.h"
#include <memory>
#include "Map.h"

enum class EngineState {
  GAME_STARTUP,
  GAME_RUNNING,
  GAME_SHUTDOWN
};

class EngineCore {
 public:
   EngineCore();
   ~EngineCore();

  void BeginGame();
  void Quit();

  static EngineCore *GetEngine();

  double deltaTime;

  u64 currentFrameTime;
  u64 lastFrameTime;

  template<typename T>
  T* SpawnObject() {
    T *object = new T();
    mObjects.push_back(object);
    return object;
  };

  void DestroyObject(GameObject* object);
  void RemoveCollider(CollisionComponent* collider);

  void DestroyAllObjects();

  void Update(float deltaTime);

  void RegisterCollider(CollisionComponent* collider);

  Map* GetMap() const;

 private:
  static EngineCore *engine;

  EngineState state;

  void SDL_Startup();
  void SDL_Shutdown();

  std::vector<GameObject*> mObjects;
  std::vector<GameObject*> mDestroyQueue;
  std::vector<CollisionComponent*> mColliders;

  void ClearDestroyQueue();
  void Remove(GameObject* object);
  void RunCollision();

  Map* mMap;
};

#endif //ENGINECORE_H
