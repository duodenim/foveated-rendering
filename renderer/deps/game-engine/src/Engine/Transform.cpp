#include "CommonTypes.h"

#include <gtc/matrix_transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <gtx/euler_angles.hpp>

Mat4 TransformToMat4(const Transform &t) {
  Mat4 transform = glm::mat4(1.0f);
  transform = glm::translate(transform, t.position);
  transform *= glm::eulerAngleXYZ(glm::radians(t.rotation.x),
                                  glm::radians(t.rotation.y),
                                  glm::radians(t.rotation.z));
  transform = glm::scale(transform, t.scale);

  return transform;
}