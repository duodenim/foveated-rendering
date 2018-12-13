#ifndef VECTOR_TYPES_H
#define VECTOR_TYPES_H
#include <glm.hpp>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef glm::vec4 Vec4;
typedef glm::vec3 Vec3;
typedef glm::vec2 Vec2;
typedef glm::mat4 Mat4;
typedef glm::ivec2 IVec2;

struct Transform {
  Vec3 position;
  Vec3 rotation;
  Vec3 scale;
};

struct Transform2D {
  Vec2 position;
  Vec2 scale;
  float rotation;
  u32 layer;
};

Mat4 TransformToMat4(const Transform &t);

#endif