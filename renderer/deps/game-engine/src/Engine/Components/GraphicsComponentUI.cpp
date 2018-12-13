#include "GraphicsComponentUI.h"
#include "../Log.h"
GraphicsComponentUI::GraphicsComponentUI() {
  mLayer = 0;

  //Setup screen scaling
  const IVec2 screen = RenderFrontend::GetScreenResolution();
  mScreenScale = ((float)(screen.x) / screen.y);
}

const Vec2 GraphicsComponentUI::GetPosition() const {
  Vec2 v;
  v.x = mModels[0].mTransform.position.x;
  v.y = mModels[0].mTransform.position.y;
  return v;
}

const float GraphicsComponentUI::GetRotation() const {
  //Only defining rotation around the z axis, as there is no reason to rotate a UI component into the background
  return mModels[0].mTransform.rotation.z;
}

const Vec2 GraphicsComponentUI::GetScale() const {
  Vec2 v;
  v.x = mModels[0].mTransform.scale.x;
  v.y = mModels[0].mTransform.scale.y;
  return v;
}

const uint8_t GraphicsComponentUI::GetLayer() {
  return mLayer;
}

void GraphicsComponentUI::SetPosition(const Vec2 position) {
  mModels[0].mTransform.position.x = position.x;
  mModels[0].mTransform.position.y = position.y;
}

void GraphicsComponentUI::SetRotation(const float rotation) {
  mModels[0].mTransform.rotation.z = rotation;
}

void GraphicsComponentUI::SetScale(const Vec2 scale) {
  mModels[0].mTransform.scale.x = scale.x / mScreenScale;
  mModels[0].mTransform.scale.y = scale.y;
}

void GraphicsComponentUI::SetLayer(const uint8_t layer) {
  if (layer > MAX_Z_LAYER) {
    Log::LogFatal("UI Components must be between layer 0 and 50");
    exit(1);
  }
  mLayer = layer;
  AdjustLayering();
}

void GraphicsComponentUI::AdjustLayering() {
  if (mModels.size() > 0) {
    if (RenderFrontend::GetDepthMode() == DEPTH_MODE::NEGATIVE_ONE_TO_ONE) {
      mModels[0].mTransform.position.z = -1 * ((float)(mLayer) / (2 * MAX_Z_LAYER));
    } else {
      mModels[0].mTransform.position.z = (float)(MAX_Z_LAYER - mLayer) / (MAX_Z_LAYER + 1);
    }
  }
}
