#include "SpriteComponent.h"
#include "../Renderer/Frontend.h"
#include "../Log.h"

SpriteComponent::SpriteComponent() {
  mSpriteRatio = 1.0f;
  mTransform.position = Vec2(0.0f, 0.0f);
  mTransform.scale = Vec2(1.0f, 1.0f);
  mTransform.rotation = 0.0f;
  mTransform.layer = 0;
  mTexture = nullptr;
}
void SpriteComponent::LoadImage(const std::string &file) {
  mTexture = RenderFrontend::LoadTexture(file);

  Log::LogInfo("Loaded Texture: " + file);

  //Setup scaling
  mSpriteRatio = (float)(mTexture->mWidth) / (mTexture->mHeight);
  mTransform.scale.x *= mSpriteRatio;
}

void SpriteComponent::ResetImage() {
   //RenderFrontend::DeleteTexture(mTexture);
   mTexture = nullptr;
}

bool SpriteComponent::HasTexture() {
  return mTexture != nullptr;
}
void SpriteComponent::Update(const float deltaTime) {
  if (mTexture != nullptr) {
    std::vector<Texture*> sprite = {mTexture};
    std::vector<Transform2D> transform = {mTransform};

    RenderFrontend::DrawSprites(sprite, transform, false);

  }
  Component::Update(deltaTime);
}
void SpriteComponent::SetPosition(const Vec2 &position) {
  mTransform.position = position;
}
void SpriteComponent::SetRotation(const float rotation) {
  mTransform.rotation = rotation;
}
void SpriteComponent::SetScale(const Vec2 &scale) {
  mTransform.scale = scale;
  mTransform.scale.x *= mSpriteRatio;
}
void SpriteComponent::SetLayer(const u32 layer) {
  mTransform.layer = layer;
}
