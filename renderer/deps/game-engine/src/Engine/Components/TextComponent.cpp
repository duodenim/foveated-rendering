#include "TextComponent.h"
#include "../FileLoader.h"

const Vec3 TEXT_SCALE = Vec3(0.1f, 0.1f, 0.1f);

TextComponent::TextComponent() {
  mFont = RenderFrontend::LoadFont("font.ttf");
  mBaseTransform.position = Vec2(0.0f, 0.0f);
  mBaseTransform.scale = Vec2(1.0f, 1.0f);
  mBaseTransform.rotation = 0.0f;
  mBaseTransform.layer = 0;
}

TextComponent::~TextComponent() {

}

u32 TextComponent::CharToModelOffset(const char c) {
  return c - ' ';
}

void TextComponent::SetText(const std::string &text) {
  mText = text;
}

const std::string TextComponent::GetText() {
  return mText;
}

void TextComponent::Update(const float deltaTime) {
  /**
   * Determine the number of characters to draw
   */
  std::string textToDraw;
  if (mScrollSpeed == 0.0f) {
    textToDraw = mText;
  } else {
    mScrollTimer += deltaTime;
    int end = ceil(mScrollTimer / mScrollSpeed) + 0.1f;
    textToDraw = mText.substr(0, end);
  }
  const Vec2 start = mBaseTransform.position;
  std::vector<Texture*> text;
  std::vector<Transform2D> transforms;
  for(int i = 0; i < textToDraw.length(); i++) {
    Character c = mFont[CharToModelOffset(textToDraw[i])];
    Transform2D t;
    t.scale.x = c.mInternalScale.x * mBaseTransform.scale.x * TEXT_SCALE.x;
    t.scale.y = c.mInternalScale.y * mBaseTransform.scale.y * TEXT_SCALE.y;
    t.position.x = start.x + (mCharOffset * i * mBaseTransform.scale.x);
    t.position.y = start.y + (c.mInternalShift.y * t.scale.y);
    t.rotation = mBaseTransform.rotation;
    t.layer = mBaseTransform.layer;
    text.push_back(c.mTexture);
    transforms.push_back(t);
  }

  RenderFrontend::DrawSprites(text, transforms, true);
}

void TextComponent::SetScrollSpeed(const float speed) {
  if (!(speed < 0.0f)) {
    mScrollSpeed = speed;
  }
}
void TextComponent::SetPosition(const Vec2 &position) {
  mBaseTransform.position = position;
}
void TextComponent::SetRotation(const float rotation) {
  mBaseTransform.rotation = rotation;
}
void TextComponent::SetScale(const Vec2 &scale) {
  mBaseTransform.scale = scale;
}
void TextComponent::SetLayer(const u32 layer) {
 mBaseTransform.layer = layer;
}
const float TextComponent::GetWidth() const {
  return mBaseTransform.scale.x * TEXT_SCALE.x * mText.length();
}
const float TextComponent::GetHeight() const {
  return mBaseTransform.scale.y * TEXT_SCALE.y;
}
const float TextComponent::GetCharacterWidth() const {
  return TEXT_SCALE.x;
}
