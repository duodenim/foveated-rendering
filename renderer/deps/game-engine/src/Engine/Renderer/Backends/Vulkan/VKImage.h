#pragma once
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include "../../../CommonTypes.h"

class VKImage {
public:
  VKImage();
  void Setup(const u32 width, const u32 height, VkImageUsageFlags usage, VkFormat format, VkImageAspectFlagBits imageAspect, VkDevice device, VmaAllocator allocator);
  void Destroy(VkDevice device, VmaAllocator allocator);
  VkImage GetImage();
  VkImageView GetImageView();
private:
  VkImage m_Image;
  VkImageView m_ImageView;
  VmaAllocation m_Memory;
};
