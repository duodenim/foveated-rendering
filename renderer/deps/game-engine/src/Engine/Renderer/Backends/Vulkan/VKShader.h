#pragma once

#include "../../Shader.h"
#include <vulkan/vulkan.h>
class VKShader : public Shader {
public:
  VkPipeline m_Pipeline;
};