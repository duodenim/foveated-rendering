#ifndef COLLISION_COMPONENT_H
#define COLLISION_COMPONENT_H

#include "../Component.h"
#include "../GameObject.h"

class CollisionComponent : public Component{
public:
  CollisionComponent();
  virtual ~CollisionComponent();
  virtual bool CheckCollision(CollisionComponent* other);
  void SetOwner(GameObject* newOwner);
  GameObject* owner;


};

#endif
