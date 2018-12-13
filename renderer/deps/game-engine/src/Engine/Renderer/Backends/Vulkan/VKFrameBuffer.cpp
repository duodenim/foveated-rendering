#include "VKFrameBuffer.h"
#include "VKError.h"

VKFrameBuffer::VKFrameBuffer() {
  m_Width = 0;
  m_Height = 0;
  m_RenderPass = VK_NULL_HANDLE;
  m_Framebuffer = VK_NULL_HANDLE;
  m_RenderPassFinishedSemaphore = VK_NULL_HANDLE;
}
void VKFrameBuffer::Setup(const u32 width,
                          const u32 height,
                          const std::vector<VkFormat> &colorFormats,
                          VkFormat depthFormat,
                          const bool depthStoreRequired,
                          VkDevice device,
                          VmaAllocator allocator) {
  //Create images for framebuffer and setup attachment descriptions
  hasDepth = depthFormat != VK_FORMAT_UNDEFINED;
  std::vector<VkImageView> imageViews(colorFormats.size());
  std::vector<VkAttachmentDescription> attachDescriptions(colorFormats.size());
  std::vector<VkAttachmentReference> attachReferences(colorFormats.size());
  m_ColorAttachments.resize(colorFormats.size());

  m_Width = width;
  m_Height = height;

  for (u32 i = 0; i < colorFormats.size(); i++) {
    m_ColorAttachments[i].Setup(width, height, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, colorFormats[i], VK_IMAGE_ASPECT_COLOR_BIT, device, allocator);
    attachDescriptions[i].format = colorFormats[i];
    attachDescriptions[i].samples = VK_SAMPLE_COUNT_1_BIT;
    attachDescriptions[i].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachDescriptions[i].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachDescriptions[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachDescriptions[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachDescriptions[i].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachDescriptions[i].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    attachReferences[i].attachment = i;
    attachReferences[i].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    imageViews[i] = m_ColorAttachments[i].GetImageView();
  }

  if (hasDepth) {
    m_DepthAttachment.Setup(width, height, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, device, allocator);

    VkAttachmentDescription depthDescription = {};
    depthDescription.format = depthFormat;
    depthDescription.samples = VK_SAMPLE_COUNT_1_BIT;
    depthDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthDescription.storeOp = depthStoreRequired ? VK_ATTACHMENT_STORE_OP_STORE : VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthDescription.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

    attachDescriptions.push_back(depthDescription);

    VkAttachmentReference depthReference = {};
    depthReference.attachment = static_cast<u32>(colorFormats.size());
    depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    attachReferences.push_back(depthReference);

    imageViews.push_back(m_DepthAttachment.GetImageView());
  }

  //Create renderpass
  VkSubpassDescription subpass = {};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = static_cast<u32>(colorFormats.size());
  subpass.pColorAttachments = attachReferences.data();
  if (hasDepth) {
    subpass.pDepthStencilAttachment = &attachReferences[attachReferences.size() - 1];
  } else {
    subpass.pDepthStencilAttachment = nullptr;
  }

  VkRenderPassCreateInfo createInfo = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
  createInfo.attachmentCount = static_cast<u32>(attachDescriptions.size());
  createInfo.pAttachments = attachDescriptions.data();
  createInfo.subpassCount = 1;
  createInfo.pSubpasses = &subpass;

  VKError::CheckResult(vkCreateRenderPass(device, &createInfo, nullptr, &m_RenderPass), "Could not make renderpass");

  //Create framebuffer
  VkFramebufferCreateInfo framebufferCreateInfo = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
  framebufferCreateInfo.width = width;
  framebufferCreateInfo.height = height;
  framebufferCreateInfo.layers = 1;
  framebufferCreateInfo.renderPass = m_RenderPass;
  framebufferCreateInfo.attachmentCount = static_cast<u32>(imageViews.size());
  framebufferCreateInfo.pAttachments = imageViews.data();

  VKError::CheckResult(vkCreateFramebuffer(device, &framebufferCreateInfo, nullptr, &m_Framebuffer), "Could not make framebuffer");

  //Create semaphore for sync
  VkSemaphoreCreateInfo semaphoreCreateInfo = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
  VKError::CheckResult(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &m_RenderPassFinishedSemaphore), "Could not make renderpass semaphore");
}
std::vector<VkImageView> VKFrameBuffer::GetColorImageViews() {
  std::vector<VkImageView> ret(m_ColorAttachments.size());

  for (u32 i = 0; i < m_ColorAttachments.size(); i++) {
    ret[i] = m_ColorAttachments[i].GetImageView();
  }

  return ret;
}
void VKFrameBuffer::Destroy(VkDevice device, VmaAllocator allocator) {
  vkDestroySemaphore(device, m_RenderPassFinishedSemaphore, nullptr);
  vkDestroyFramebuffer(device, m_Framebuffer, nullptr);
  vkDestroyRenderPass(device, m_RenderPass, nullptr);
  for (auto &image : m_ColorAttachments) {
    image.Destroy(device, allocator);
  }
  if (hasDepth) {
    m_DepthAttachment.Destroy(device, allocator);
  }
}
VkFramebuffer VKFrameBuffer::GetFramebuffer() {
  return m_Framebuffer;
}
VkRenderPass VKFrameBuffer::GetRenderPass() {
  return m_RenderPass;
}
VkSemaphore VKFrameBuffer::GetSemaphore() {
  return m_RenderPassFinishedSemaphore;
}
u32 VKFrameBuffer::GetWidth() {
  return m_Width;
}
u32 VKFrameBuffer::GetHeight() {
  return m_Height;
}
VkImageView VKFrameBuffer::GetDepthImageView() {
  return m_DepthAttachment.GetImageView();
}
std::vector<VkDescriptorImageInfo> VKFrameBuffer::GetColorImageInfos(VkSampler sampler) {
  std::vector<VkDescriptorImageInfo> imageInfos(m_ColorAttachments.size());

  for (u32 i = 0; i < m_ColorAttachments.size(); i++) {
    imageInfos[i].imageView = m_ColorAttachments[i].GetImageView();
    imageInfos[i].sampler = sampler;
    imageInfos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  }

  return imageInfos;
}
VkDescriptorImageInfo VKFrameBuffer::GetDepthImageInfo(VkSampler sampler) {
  VkDescriptorImageInfo imageInfo = {};
  imageInfo.sampler = sampler;
  imageInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
  imageInfo.imageView = m_DepthAttachment.GetImageView();
  return imageInfo;
}
