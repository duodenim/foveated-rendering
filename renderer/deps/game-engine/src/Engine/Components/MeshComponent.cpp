#include "MeshComponent.h"
#include "../Renderer/Frontend.h"
#include "../Log.h"

MeshComponent::MeshComponent() {
  mVisible = true;
  mCacheNeedsUpdate = false;
  mCachedTransform.position = Vec3(0.0f);
  mCachedTransform.rotation = Vec3(0.0f);
  mCachedTransform.scale = Vec3(1.0f, 1.0f, 1.0f);
  mTransformMatrix = Mat4(1.0f);
}
void MeshComponent::LoadMeshData(const std::string & file) {
  mModel = RenderFrontend::LoadModel(file);
  Log::LogInfo("Loaded Mesh: " + file);
}
void MeshComponent::LoadShader(const std::string &vertexShaderFile, const std::string &fragmentShaderFile) {
  mModel.mShader = RenderFrontend::LoadShader(vertexShaderFile, fragmentShaderFile, DRAW_STAGE::WORLD);
}
void MeshComponent::SetTexture(const std::string &textureFile) {
  Texture *t = RenderFrontend::LoadTexture(textureFile);

  //for(auto &m : mModels) {
//    m.mTexture = t;
  //}

  Log::LogInfo("Loaded Texture: " + textureFile);
}
void MeshComponent::SetUserData(const Mat4 &value) {
  RenderFrontend::SetShaderUserData(value);
}
void MeshComponent::Update(const float deltaTime) {
  if (mCacheNeedsUpdate) {
    mTransformMatrix = TransformToMat4(mCachedTransform);
    mCacheNeedsUpdate = false;
  }
  if (mVisible) {
    RenderFrontend::Draw(mModel, mTransformMatrix);
  }
  Component::Update(deltaTime);
}
const Vec3 MeshComponent::GetPosition(const int index) const {
  return mCachedTransform.position;
}
const Vec3 MeshComponent::GetRotation(const int index) const {
  return mCachedTransform.rotation;
}
const Vec3 MeshComponent::GetScale(const int index) const {
  return mCachedTransform.scale;
}
const bool MeshComponent::GetVisibility() const {
  return mVisible;
}
void MeshComponent::SetTransform(const Transform transform) {
  mCachedTransform = transform;
  mCacheNeedsUpdate = true;
}
void MeshComponent::SetPosition(const Vec3 position) {
  mCachedTransform.position = position;
  mCacheNeedsUpdate = true;
}
void MeshComponent::SetRotation(const Vec3 rotation) {
  mCachedTransform.rotation = rotation;
  mCacheNeedsUpdate = true;
}
void MeshComponent::SetScale(const Vec3 scale) {
  mCachedTransform.scale = scale;
  mCacheNeedsUpdate = true;
}
void MeshComponent::SetVisibility(const bool visibility) {
  mVisible = visibility;
}
