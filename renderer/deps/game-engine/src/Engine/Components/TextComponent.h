#ifndef TEXTCOMPONENT_H
#define TEXTCOMPONENT_H

#include "../Component.h"
#include "../Renderer/Frontend.h"

class TextComponent : public Component {
public:
  TextComponent();
  ~TextComponent();
  void Update(const float deltaTime);
  void SetText(const std::string &text);
  void SetScrollSpeed(const float speed);
  void SetPosition(const Vec2 &position);
  void SetRotation(const float rotation);
  void SetScale(const Vec2 &scale);
  void SetLayer(const u32 layer);
  const std::string GetText();
  const float GetWidth() const;
  const float GetHeight() const;
  const float GetCharacterWidth() const;
private:
  u32 CharToModelOffset(const char c);
  std::vector<Character> mFont;
  std::string mText;
  float mCharOffset = 0.1f;
  float mScrollSpeed = 0.0f; //The scroll speed of this text, measured in seconds per character
  float mScrollTimer = 0.0f; //Amount of time since the text began drawing, used to determine how many characters to draw
  Transform2D mBaseTransform;
};

#endif