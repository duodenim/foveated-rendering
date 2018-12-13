#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include <vector>
#include "Component.h"

struct InputCallbackData;
class GameObject {
public:
  GameObject();
  virtual ~GameObject();
  virtual void Update(float deltaTime);
  virtual void Collision(GameObject* other);
  virtual void Input(const InputCallbackData &inputData);
  void Destroy();

  template<typename T>
  std::shared_ptr<T> AddComponent() {
    std::shared_ptr<T> component = std::make_shared<T>();
    mComponents.push_back(component);
    return component;
  }

private:
  std::vector<std::shared_ptr<Component>> mComponents;
};

#endif
