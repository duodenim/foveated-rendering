#pragma once

#include "../Component.h"
#include "../Renderer/Model.h"
#include "../CommonTypes.h"

class MeshComponent : public Component {
public:
  MeshComponent();
  void Update(const float deltaTime) override;
  void LoadMeshData(const std::string &file);
  void LoadShader(const std::string &vertexShaderFile, const std::string &fragmentShaderFile);
  void SetTexture(const std::string &textureFile);
  void SetUserData(const Mat4 &value);
  const Vec3 GetPosition(const int index) const;
  const Vec3 GetRotation(const int index) const;
  const Vec3 GetScale(const int index) const;
  const bool GetVisibility() const;
  void SetTransform(const Transform transform);
  void SetPosition(const Vec3 position);
  void SetRotation(const Vec3 rotation);
  void SetScale(const Vec3 scale);
  void SetVisibility(const bool visibility);
private:
  bool mVisible;
  ModelTree mModel;
  Mat4 mTransformMatrix;
  Transform mCachedTransform; //Transform copy of root node's model matrix for easier access
  bool mCacheNeedsUpdate;     //Set when the cached transform and model matrix are out of date
};
