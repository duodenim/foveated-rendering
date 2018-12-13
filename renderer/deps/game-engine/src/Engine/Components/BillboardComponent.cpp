#include "BillboardComponent.h"
#include "../Renderer/Frontend.h"
#include "../Log.h"
#include "../InputManager.h"
#include <gtc/matrix_transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <gtx/euler_angles.hpp>

BillboardComponent::BillboardComponent() {
  mCachedTransform.position = Vec3(0.0f, 0.0f, 0.0f);
  mCachedTransform.rotation = Vec3(0.0f, 0.0f, 0.0f);
  mCachedTransform.scale = Vec3(1.0f, 1.0f, 1.0);
  mTransformMatrix = Mat4(1.0f);

  mCacheNeedsUpdate = false;

  mModel = RenderFrontend::LoadModel("models/sprite.obj");
  mModel.mShader = RenderFrontend::LoadShader("billboard.vert", "billboard.frag", DRAW_STAGE::WORLD);
  mSpriteRatio = 1.0f;
}
const Vec3 BillboardComponent::GetPosition(const int index) const {
  return mCachedTransform.position;
}
const Vec3 BillboardComponent::GetRotation(const int index) const {
  return mCachedTransform.rotation;
}
const Vec3 BillboardComponent::GetScale(const int index) const {
  return mCachedTransform.scale;
}

void BillboardComponent::Update(const float deltaTime) {
  /*static float time = 0;
  time += 10 * deltaTime;

  if (Input::GetKeyState(SDL_SCANCODE_T)) {
    time = 0.0f;
  }

  float factor = 1 / (time);

  if (factor < 0.03f) {
    factor = 0.0f;
  }

  SetScale(Vec3(1.0f, factor * sinf(time) + (1 - factor), 1.0f));
  SetPosition(Vec3(0.0f, factor * sinf(time) - factor, -3.0f));*/
  if (mCacheNeedsUpdate) {
    mTransformMatrix = glm::mat4(1.0f);
    mTransformMatrix = glm::translate(mTransformMatrix, mCachedTransform.position);
    mTransformMatrix *= glm::eulerAngleXYZ(glm::radians(mCachedTransform.rotation.x),
                                           glm::radians(mCachedTransform.rotation.y),
                                           glm::radians(mCachedTransform.rotation.z));
    mTransformMatrix = glm::scale(mTransformMatrix, mCachedTransform.scale);
    mCacheNeedsUpdate = false;
  }
  RenderFrontend::Draw(mModel, mTransformMatrix);
  Component::Update(deltaTime);
}
void BillboardComponent::SetImage(const std::string &imageFile) {
  if (mModel.mMeshes.size() == 0) {
    Log::LogWarning("Set image called on an empty model");
    Log::LogWarning(imageFile);
  } else {
    mModel.mMeshes[0].mTexture = RenderFrontend::LoadTexture(imageFile);

    //Setup scaling
    mSpriteRatio = (float)mModel.mMeshes[0].mTexture->mWidth / mModel.mMeshes[0].mTexture->mHeight;

    mCachedTransform.scale.x *= mSpriteRatio;
    mCacheNeedsUpdate = true;
  }
}
void BillboardComponent::SetPosition(const Vec3 position) {
  mCachedTransform.position = position;
  mCacheNeedsUpdate = true;
}
void BillboardComponent::SetRotation(const Vec3 rotation) {
  mCachedTransform.rotation = rotation;
  mCacheNeedsUpdate = true;
}
void BillboardComponent::SetScale(const Vec3 scale) {
  mCachedTransform.scale = scale;
  mCachedTransform.scale.x *= mSpriteRatio;
  mCacheNeedsUpdate = true;
}