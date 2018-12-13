#include "Component.h"

Component::~Component() {

}
void Component::Update(const float deltaTime) {
  for(auto& child : mChildren) {
    child->Update(deltaTime);
  }
}
