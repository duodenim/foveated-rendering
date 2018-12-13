#pragma once

#include "VKImage.h"
#include <vector>

class VKFrameBuffer {
public:
  VKFrameBuffer();
  void Setup(const u32 width, const u32 height, const std::vector<VkFormat> &colorFormats, VkFormat depthFormat, const bool depthStoreRequired, VkDevice device, VmaAllocator allocator);
  void Destroy(VkDevice device, VmaAllocator allocator);
  u32 GetWidth();
  u32 GetHeight();
  VkFramebuffer GetFramebuffer();
  VkRenderPass GetRenderPass();
  VkSemaphore GetSemaphore();
  std::vector<VkImageView> GetColorImageViews();
  VkImageView GetDepthImageView();
  std::vector<VkDescriptorImageInfo> GetColorImageInfos(VkSampler sampler);
  VkDescriptorImageInfo GetDepthImageInfo(VkSampler sampler);
private:
  u32 m_Width;
  u32 m_Height;
  VkRenderPass m_RenderPass;
  VkFramebuffer m_Framebuffer;

  std::vector<VKImage> m_ColorAttachments;
  VKImage m_DepthAttachment;

  VkSemaphore m_RenderPassFinishedSemaphore;

  bool hasDepth;
};
