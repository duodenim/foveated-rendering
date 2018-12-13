#pragma once

#include "Types.h"
#include "Texture.h"
#include "Shader.h"

#include <memory>
#include <vector>

class VertexBuffer {
};

class Model {
public:
  Model() : mVBuffer(nullptr), mTexture(nullptr) {}
  VertexBuffer* mVBuffer;
  Texture * mTexture;
  u32 mNumFaces;
};

class Node {
public:
  Mat4 mTransformMatrix;
  std::vector<u32> mMeshIndices;
  std::vector<std::shared_ptr<Node>> mChildren;
};

class ModelTree {
public:
  Shader* mShader;
  std::shared_ptr<Node> mRoot;
  std::vector<Model> mMeshes;
};

struct Character {
  Texture* mTexture;
  Vec2 mInternalScale;
  Vec2 mInternalShift;
};