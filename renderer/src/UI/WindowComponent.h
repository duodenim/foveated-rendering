#pragma once

#include <Component.h>
#include <CommonTypes.h>

class TextComponent;
class SpriteComponent;

class WindowComponent : public Component {
public:
  WindowComponent();
  void SetBaseLayer(const uint8_t &layer);
  void SetCenter(const Vec2 &center);

  /**
   * Sets a (fixed) width for this window
   * @param width The new (fixed) width of the window, setting to 0.0 will turn on automatic scaling again
   */
  void SetWidth(const float width);

  /**
   * Sets a (fixed) height for this window
   * @param height The new height of the window, setting to 0.0 will enable automatic scaling
   */
  void SetHeight(const float height);
  void SetText(const std::string &text);
private:
  std::shared_ptr<TextComponent> mContent;
  std::shared_ptr<SpriteComponent> mBackground;

  float GetBackgroundWidth() const;
  float GetBackgroundHeight() const;
  Vec2 GetTextStart() const;

  float mWidth;
  float mHeight;
  uint8_t mBaseLayer;
  Vec2 mCenter;
  float mTextScale;
};
