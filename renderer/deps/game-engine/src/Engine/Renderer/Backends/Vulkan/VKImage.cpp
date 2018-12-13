#include "VKImage.h"
VKImage::VKImage() {
  m_Image = VK_NULL_HANDLE;
  m_ImageView = VK_NULL_HANDLE;
}
void VKImage::Setup(const u32 width,
                    const u32 height,
                    VkImageUsageFlags usage,
                    VkFormat format,
                    VkImageAspectFlagBits imageAspect,
                    VkDevice device,
                    VmaAllocator allocator) {
  //Create image handle and allocate memory
  VkImageCreateInfo imageCreateInfo = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
  imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
  imageCreateInfo.extent.width = width;
  imageCreateInfo.extent.height = height;
  imageCreateInfo.extent.depth = 1;
  imageCreateInfo.mipLevels = 1;
  imageCreateInfo.arrayLayers = 1;
  imageCreateInfo.format = format;
  imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
  imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  imageCreateInfo.usage = usage;
  imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;

  VmaAllocationCreateInfo allocationCreateInfo = {};
  allocationCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
  vmaCreateImage(allocator, &imageCreateInfo, &allocationCreateInfo, &m_Image, &m_Memory, nullptr);

  //Create image view handle
  VkImageViewCreateInfo imageViewCreateInfo = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
  imageViewCreateInfo.image = m_Image;
  imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
  imageViewCreateInfo.format = format;
  imageViewCreateInfo.subresourceRange.aspectMask = imageAspect;
  imageViewCreateInfo.subresourceRange.layerCount = 1;
  imageViewCreateInfo.subresourceRange.levelCount = 1;
  imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
  imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
  vkCreateImageView(device, &imageViewCreateInfo, nullptr, &m_ImageView);
}
VkImage VKImage::GetImage() {
  return m_Image;
}
VkImageView VKImage::GetImageView() {
  return m_ImageView;
}
void VKImage::Destroy(VkDevice device, VmaAllocator allocator) {
  vkDestroyImageView(device, m_ImageView, nullptr);
  vmaDestroyImage(allocator, m_Image, m_Memory);
}
