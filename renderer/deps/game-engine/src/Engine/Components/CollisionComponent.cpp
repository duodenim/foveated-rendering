#include "CollisionComponent.h"
#include "../EngineCore.h"

CollisionComponent::CollisionComponent() {
  owner = NULL;
  EngineCore::GetEngine()->RegisterCollider(this);
}
CollisionComponent::~CollisionComponent() {
  EngineCore::GetEngine()->RemoveCollider(this);
}
bool CollisionComponent::CheckCollision(CollisionComponent* other) {
  return false;

}

void CollisionComponent::SetOwner(GameObject* newOwner) {
  owner = newOwner;
}
