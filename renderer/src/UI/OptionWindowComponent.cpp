#include "OptionWindowComponent.h"

const Vec2 TEXT_START_OFFSET = Vec2(0.1f, -0.1f);

const Vec2 CURSOR_OFFSET = Vec2(-0.05, 0.0);

const float HEIGHT_OFFSET = 0.05f;

OptionWindowComponent::OptionWindowComponent() {
  mNumColumns = 1;
  mColumnWidth = 0.0f;
  mColumnHeight = 0.0f;
  mBackground.LoadImage("ui/window_background.png");
  mTextScale = 0.5f;

  mCursor.LoadImage("ui/selector.png");
  mCursor.SetScale(Vec2(0.05, 0.05));
  mCursor.SetLayer(1);
  mCursorPosition = 0;

  mSelectSound.LoadWAVFromFile("sfx/select.wav");
  mSelectSound.SetVolume(0.3f);

  mCursorMoveSound.LoadWAVFromFile("sfx/cursor_move.wav");
  mCursorMoveSound.SetVolume(0.5f);
}

void OptionWindowComponent::SetNumColumns(const int &columns) {
  mNumColumns = columns;
}

void OptionWindowComponent::SetCenter(const Vec2 &center) {
  mCenter = center;
  mBackground.SetPosition(center);
}

void OptionWindowComponent::AddOption(const std::string &text) {
  TextComponent t;
  t.SetText(text);
  t.SetScale(Vec2(mTextScale, mTextScale));
  t.SetLayer(mBackground.GetLayer() + 1);

  /**
   * Setup column to scale with the widest text in the window
   */
  const float w = t.GetWidth() + TEXT_START_OFFSET.x;
  if (w > mColumnWidth) {
    mColumnWidth = w;
  }
  mColumnHeight = t.GetHeight() + HEIGHT_OFFSET;
  mOptions.push_back(t);
}

void OptionWindowComponent::Draw(const float deltaTime) {
  int numRows = ceil(((float)mOptions.size()) / mNumColumns) + 0.01f;
  float windowHeight = numRows * mColumnHeight;

  if (numRows > 1) {
    windowHeight -= (numRows - 1) * HEIGHT_OFFSET;
  }
  mBackground.SetScale(Vec2(mNumColumns * mColumnWidth, windowHeight));
  mBackground.Draw();
  const Vec2 start = mCenter - Vec2(mBackground.GetScale().x, -mBackground.GetScale().y) + TEXT_START_OFFSET;

  for (int i = 0; i < mOptions.size(); i++) {
    int row = i / mNumColumns;
    int column = i % mNumColumns;
    mOptions[i].SetPosition(Vec2(start.x + column * mColumnWidth, start.y - row * mColumnHeight));
    mOptions[i].Draw(deltaTime);

    if (mCursorPosition == i) {
      mCursor.SetPosition(Vec2(start.x + column * mColumnWidth, start.y - row * mColumnHeight) + CURSOR_OFFSET);
      mCursor.Draw();
    }
  }
}

void OptionWindowComponent::SetBaseLayer(const uint8_t layer) {
  mBackground.SetLayer(layer);
  mCursor.SetLayer(layer + 1);
  for (auto &t : mOptions) {
    t.SetLayer(layer + 1);
  }
}

void OptionWindowComponent::MoveCursor(const CursorDirection direction) {
  mCursorMoveSound.Play();
  switch (direction) {
    case CursorDirection::RIGHT: {
      if (((mCursorPosition + 1) % mNumColumns) != 0 && (mCursorPosition + 1) < mOptions.size()) {
        mCursorPosition++;
      }
      break;
    }
    case CursorDirection::LEFT: {
      if (mCursorPosition % mNumColumns != 0) {
        mCursorPosition--;
      }
      break;
    }
    case CursorDirection::DOWN: {
      if (mCursorPosition + mNumColumns < mOptions.size()) {
        mCursorPosition += mNumColumns;
      }
      break;
    }
    case CursorDirection::UP: {
      if (mCursorPosition - mNumColumns >= 0) {
        mCursorPosition -= mNumColumns;
      }
      break;
    }
  }
}

const std::string OptionWindowComponent::Select() {
  mSelectSound.Play();
  return mOptions[mCursorPosition].GetText();
}

void OptionWindowComponent::Clear() {
  mColumnWidth = 0.0f;
  mCursorPosition = 0;
  mOptions.clear();
}




