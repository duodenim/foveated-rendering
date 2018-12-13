#pragma once

#include "Model.h"
#include "FrameBuffer.h"
#include <vector>
#include <string>
#include "Light.h"

enum class DEPTH_MODE {
  ZERO_TO_ONE,
  NEGATIVE_ONE_TO_ONE
};

enum class DRAW_STAGE {
  WORLD,
  UI,
  ASPECT,
  SHADOW,
  FOVEATED
};

class Drawable {
public:
  Shader* mShader;
  Texture* mTexture;
  Mat4 mTransformMatrix;
  VertexBuffer* mVBuffer;
  u32 mNumFaces;
};

class RenderBackend {
public:
  virtual void Init() = 0;
  virtual void WindowInit(const std::string name, int width, const int height) = 0;
  virtual void Shutdown() = 0;
  virtual const Model LoadModel(const std::vector<Vertex> vertices, const std::vector<u32> indices) = 0;
  virtual Texture* LoadTexture(const unsigned char* data, const int width, const int height, const int numChannels) = 0;
  virtual Shader* CreateShader(const std::vector<char> vertexProgram, const std::vector<char> fragmentProgram, const DRAW_STAGE stage) = 0;
  virtual void SetFrameBufferModel(const Model &model) = 0;
  virtual void SetFrameBufferShader(Shader* shader, const DRAW_STAGE stage) = 0;
  virtual void Draw(const Mat4 &viewMatrix, const Mat4 &projMatrix, const Mat4 &userData, const std::vector<Drawable> &scene, const std::vector<Drawable> &ui, const LightData& lights) = 0;

  virtual void DeleteModel(Model &model) = 0;
  virtual void DeleteTexture(Texture* tex) = 0;
  virtual void DeleteShader(Shader* shader) = 0;

  virtual std::string GetShaderFolderName() = 0;
  virtual DEPTH_MODE GetDepthMode() = 0;

  virtual std::string GetDeviceName() = 0;
  virtual u64 GetUsedVRAM() = 0;
};