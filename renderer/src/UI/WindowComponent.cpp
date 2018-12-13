#include "WindowComponent.h"
#include <Components/SpriteComponent.h>
#include <Components/TextComponent.h>

const uint8_t LAYER_DIFF = 1;
const float SCROLL_SPEED = 0.05f;

WindowComponent::WindowComponent() {
  mBaseLayer = 0;
  mBackground = AddChild<SpriteComponent>();
  mContent = AddChild<TextComponent>();

  mBackground->LoadImage("ui/window_background.png");
  mTextScale = 1.0f;
  mWidth = 0.0f;
  mHeight = 0.0f;
}

void WindowComponent::SetBaseLayer(const uint8_t &layer) {
  mBaseLayer = layer;
  mContent->SetLayer(mBaseLayer + LAYER_DIFF);
  mBackground->SetLayer(layer);
}

void WindowComponent::SetCenter(const Vec2 &center) {
  mCenter = center;
  mBackground->SetPosition(mCenter);
  mContent->SetPosition(GetTextStart());
}

void WindowComponent::SetWidth(const float width) {
  mWidth = width;
  mBackground->SetScale(Vec2(GetBackgroundWidth() / 2, GetBackgroundHeight()));
  mContent->SetPosition(GetTextStart());
}

void WindowComponent::SetHeight(const float height) {
  mHeight = height;
  mBackground->SetScale(Vec2(GetBackgroundWidth() / 2, GetBackgroundHeight()));
  mContent->SetPosition(GetTextStart());
}


void WindowComponent::SetText(const std::string &text) {

  mContent->SetText(text);
  mContent->SetLayer(mBaseLayer + LAYER_DIFF);

  /**
   * Scale text by x scale on both axes to prevent distortion
   */
  mContent->SetScale(Vec2(mTextScale, mTextScale));

  /**
   * Adjust background to fit
   */
   float bWidth = GetBackgroundWidth();
   float bHeight = GetBackgroundHeight();
  mBackground->SetScale(Vec2(GetBackgroundWidth() / 2, GetBackgroundHeight()));
  mContent->SetPosition(GetTextStart());
  mContent->SetScrollSpeed(SCROLL_SPEED);
}

float WindowComponent::GetBackgroundWidth() const {
  if (mWidth == 0.0f) {
    return mContent->GetWidth();
  } else {
    return mWidth;
  }
}

float WindowComponent::GetBackgroundHeight() const {
  if (mHeight == 0.0f) {
    return mContent->GetHeight();
  } else {
    return mHeight;
  }
}
Vec2 WindowComponent::GetTextStart() const {
  return Vec2((-0.5f * GetBackgroundWidth()) + (mContent->GetCharacterWidth() / 2), 0.0f) + mCenter;
}




