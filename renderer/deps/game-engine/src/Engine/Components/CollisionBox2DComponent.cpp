//
// Created by joseph on 12/31/16.
//

#include "CollisionBox2DComponent.h"

CollisionBox2DComponent::CollisionBox2DComponent() {

}
CollisionBox2DComponent::~CollisionBox2DComponent() {

}
void CollisionBox2DComponent::SetSize(float newWidth, float newHeight) {
  width = newWidth;
  height = newHeight;
}
bool CollisionBox2DComponent::CheckCollision(CollisionComponent *other) {
  CollisionBox2DComponent* otherBox = dynamic_cast<CollisionBox2DComponent*>(other);
  if (otherBox == NULL) {
    return false;
  }
  if (abs(otherBox->position.z - position.z) > SIZE_Z) {
    return false;
  }
  bool collision = position.x + (width / 2.0f) > otherBox->position.x - (otherBox->width / 2.0f);
  collision &= otherBox->position.x + (otherBox->width / 2.0f) > position.x - (width / 2.0f);

  collision &= position.y + (height / 2.0f) > otherBox->position.y - (otherBox->height / 2.0f);
  collision &= otherBox->position.y + (otherBox->height / 2.0f) > position.y - (height/ 2.0f);
  
  return collision;
}
