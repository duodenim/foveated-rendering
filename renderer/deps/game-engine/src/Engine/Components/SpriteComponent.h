#pragma once

#include "../Component.h"
#include "../CommonTypes.h"

class Texture;

class SpriteComponent : public Component {
public:
  SpriteComponent();
  void Update(const float deltaTime);
  void LoadImage(const std::string &file);
  void SetPosition(const Vec2 &position);
  void SetRotation(const float rotation);
  void SetScale(const Vec2 &scale);
  void SetLayer(const u32 layer);
  void ResetImage();
  bool HasTexture();
protected:
  float mSpriteRatio;
  Transform2D mTransform;
  Texture* mTexture;
};
