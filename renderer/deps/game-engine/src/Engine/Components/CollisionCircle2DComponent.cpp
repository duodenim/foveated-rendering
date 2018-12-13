#include "CollisionCircle2DComponent.h"
#include <cmath>

CollisionCircle2DComponent::CollisionCircle2DComponent() {

}

CollisionCircle2DComponent::~CollisionCircle2DComponent() {
}

void CollisionCircle2DComponent::SetRadius(float newRadius) {
  radius = newRadius;
}

bool CollisionCircle2DComponent::CheckCollision(CollisionComponent * other){

  CollisionCircle2DComponent* otherCircle = dynamic_cast<CollisionCircle2DComponent*>(other);
  if (otherCircle == NULL) {
    return false;
  }
  if (abs(otherCircle->position.z - position.z) > SIZE_Z) {
    return false;
  }
  float dx = otherCircle->position.x - this->position.x;
  float dy = otherCircle->position.y - this->position.y;

  float distance = sqrt(pow(dx, 2) + pow(dy, 2));

  return (distance < (this->radius + otherCircle->radius));
}
