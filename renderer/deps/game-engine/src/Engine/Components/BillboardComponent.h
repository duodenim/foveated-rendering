#pragma once

#include "../Component.h"
#include "../Renderer/Model.h"

class BillboardComponent : public Component {
public:
  BillboardComponent();
  void Update(const float deltaTime);
  const Vec3 GetPosition(const int index) const;
  const Vec3 GetRotation(const int index) const;
  const Vec3 GetScale(const int index) const;
  void SetImage(const std::string &imageFile);
  void SetPosition(const Vec3 position);
  void SetRotation(const Vec3 rotation);
  void SetScale(const Vec3 scale);
private:
  ModelTree mModel;
  Mat4 mTransformMatrix;
  Transform mCachedTransform; //Transform copy of root node's model matrix for easier access
  bool mCacheNeedsUpdate;     //Set when the cached transform and model matrix are out of date
  float mSpriteRatio;
};
