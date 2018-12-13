#include "GameObject.h"
#include "EngineCore.h"

GameObject::GameObject() {

}
GameObject::~GameObject() {

}

void GameObject::Collision(GameObject * other) {
}

void GameObject::Input(const InputCallbackData &inputData) {
}

void GameObject::Destroy() {
  EngineCore::GetEngine()->DestroyObject(this);
}
void GameObject::Update(float deltaTime) {
  for(auto& component : mComponents) {
    component->Update(deltaTime);
  }
}
