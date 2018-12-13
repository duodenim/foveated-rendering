#pragma once

#include "../../Model.h"
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

class VKVertexBuffer : public VertexBuffer {
public:
  VkBuffer m_Buffer;
  VmaAllocation m_Allocation;
  VkDeviceSize m_IndexOffset;
};