#include "VKSurface.h"
#include "../../../CommonTypes.h"
#include "../../../Log.h"
#include <SDL_vulkan.h>
#include "VKError.h"

VKSurface::VKSurface() {
  m_Window = nullptr;
  m_Surface = VK_NULL_HANDLE;
  m_SurfaceCapabilities = {};
  m_SwapchainExtent.height = 0;
  m_SwapchainExtent.width = 0;
  m_PresentRenderPass = VK_NULL_HANDLE;
}

void VKSurface::CreateWindow(const std::string &windowName, const int width, const int height) {
  m_Window = SDL_CreateWindow(windowName.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_VULKAN | SDL_WINDOW_FULLSCREEN);

  if (m_Window == nullptr) {
    Log::LogFatal("[VKBackend] Could not create window");
  }
}

std::vector<const char *> VKSurface::GetInstanceExtensions() {
  u32 extensionCount = 0;

  SDL_Vulkan_GetInstanceExtensions(m_Window, &extensionCount, nullptr);
  std::vector<const char *> extensions(extensionCount);
  SDL_Vulkan_GetInstanceExtensions(m_Window, &extensionCount, extensions.data());

  return extensions;
}

void VKSurface::CreateSurface(VkInstance instance) {
  SDL_Vulkan_CreateSurface(m_Window, instance, &m_Surface);
}

VkSurfaceKHR VKSurface::GetSurface() {
  return m_Surface;
}

VkSurfaceCapabilitiesKHR VKSurface::GetCapabilities() {
  return m_SurfaceCapabilities;
}

VkSurfaceFormatKHR VKSurface::SelectSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &formats) {
  //If only format is undefined, pick a good one
  VkSurfaceFormatKHR returnFormat = formats[0];

  if (formats.size() == 1 && formats[0].format == VK_FORMAT_UNDEFINED) {
    returnFormat.format = VK_FORMAT_B8G8R8A8_UNORM;
    returnFormat.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
  } else {
    for (const auto &format : formats) {
      if (format.format == VK_FORMAT_B8G8R8A8_UNORM && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
        returnFormat = format;
        break;
      }
    }
  }

  return returnFormat;
}
VkSurfaceFormatKHR VKSurface::GetDefaultFormat() {
  return m_DefaultSurfaceFormat;
}
void VKSurface::CreateSwapchain(VkPhysicalDevice physicalDevice, VkDevice device) {

  //Fill in formats
  u32 surfaceFormatCount = 0;
  vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, m_Surface, &surfaceFormatCount, nullptr);
  m_SurfaceFormats.resize(surfaceFormatCount);
  vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, m_Surface, &surfaceFormatCount, m_SurfaceFormats.data());

  //Fill in capabilities
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, m_Surface, &m_SurfaceCapabilities);

  //Fill in present modes
  u32 presentModeCount = 0;
  vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, m_Surface, &presentModeCount, nullptr);
  m_SurfacePresentModes.resize(presentModeCount);
  vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, m_Surface, &presentModeCount, m_SurfacePresentModes.data());

  //Select a good default surface format
  m_DefaultSurfaceFormat = SelectSurfaceFormat(m_SurfaceFormats);

  m_SwapchainExtent = m_SurfaceCapabilities.minImageExtent;

  //Create swapchain
  VkSwapchainCreateInfoKHR scCreateInfo = {VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
  scCreateInfo.surface = m_Surface;
  scCreateInfo.minImageCount = m_SurfaceCapabilities.minImageCount;
  scCreateInfo.imageFormat = m_DefaultSurfaceFormat.format;
  scCreateInfo.imageColorSpace = m_DefaultSurfaceFormat.colorSpace;
  scCreateInfo.imageExtent = m_SurfaceCapabilities.minImageExtent;
  scCreateInfo.imageArrayLayers = 1;
  scCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  scCreateInfo.queueFamilyIndexCount = 0;
  scCreateInfo.pQueueFamilyIndices = nullptr;
  scCreateInfo.preTransform = m_SurfaceCapabilities.currentTransform;
  scCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  scCreateInfo.presentMode = SelectPresentMode(m_SurfacePresentModes);
  scCreateInfo.clipped = VK_TRUE;
  scCreateInfo.oldSwapchain = VK_NULL_HANDLE;

  VKError::CheckResult(vkCreateSwapchainKHR(device, &scCreateInfo, nullptr, &m_Swapchain), "Could not create swapchain");

  //Get image handles
  u32 scImageCount = 0;
  vkGetSwapchainImagesKHR(device, m_Swapchain, &scImageCount, nullptr);
  m_SwapchainImages.resize(scImageCount);
  vkGetSwapchainImagesKHR(device, m_Swapchain, &scImageCount, m_SwapchainImages.data());

  //Create image view handles
  m_SwapchainImageViews.resize(scImageCount);
  for (u32 i = 0; i < m_SwapchainImages.size(); i++) {
    VkImageViewCreateInfo imageViewCreateInfo = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
    imageViewCreateInfo.image = m_SwapchainImages[i];
    imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imageViewCreateInfo.format = m_DefaultSurfaceFormat.format;
    imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;

    imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageViewCreateInfo.subresourceRange.levelCount = 1;
    imageViewCreateInfo.subresourceRange.layerCount = 1;
    imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
    imageViewCreateInfo.subresourceRange.baseMipLevel = 0;

    vkCreateImageView(device, &imageViewCreateInfo, nullptr, &m_SwapchainImageViews[i]);
  }

  //Create renderpass
  {
    VkAttachmentDescription colorAttachDesc = {};
    colorAttachDesc.format = GetDefaultFormat().format;
    colorAttachDesc.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachDesc.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachRef = {};
    colorAttachRef.attachment = 0;
    colorAttachRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpassDescription = {};
    subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_BEGIN_RANGE;
    subpassDescription.colorAttachmentCount = 1;
    subpassDescription.pColorAttachments = &colorAttachRef;

    //Presentation needs a dependency to wait for the image to actually be available before doing
    //the layout transition
    VkSubpassDependency presentDependency = {};
    presentDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    presentDependency.dstSubpass = 0;
    presentDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    presentDependency.srcAccessMask = 0;
    presentDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    presentDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo renderPassCreateInfo = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
    renderPassCreateInfo.attachmentCount = 1;
    renderPassCreateInfo.pAttachments = &colorAttachDesc;
    renderPassCreateInfo.subpassCount = 1;
    renderPassCreateInfo.pSubpasses = &subpassDescription;
    renderPassCreateInfo.dependencyCount = 1;
    renderPassCreateInfo.pDependencies = &presentDependency;

    VKError::CheckResult(vkCreateRenderPass(device, &renderPassCreateInfo, nullptr, &m_PresentRenderPass), "Could not make present render pass");
  }

  //Create framebuffers
  {
    m_Framebuffers.resize(GetSwapchainImageCount());

    for (u32 i = 0; i < GetSwapchainImageCount(); i++) {

      VkFramebufferCreateInfo fbCreateInfo = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
      fbCreateInfo.renderPass = m_PresentRenderPass;
      fbCreateInfo.attachmentCount = 1;
      fbCreateInfo.pAttachments = &m_SwapchainImageViews[i];
      fbCreateInfo.width = m_SwapchainExtent.width;
      fbCreateInfo.height = m_SwapchainExtent.height;
      fbCreateInfo.layers = 1;

      VKError::CheckResult(vkCreateFramebuffer(device, &fbCreateInfo, nullptr, &m_Framebuffers[i]), "Could not create present framebuffer");
    }
  }
}
u32 VKSurface::GetSwapchainImageCount() {
  return (u32)m_SwapchainImages.size();
}
VkExtent2D VKSurface::GetSwapchainExtent() {
  return m_SwapchainExtent;
}
std::vector<VkImageView> VKSurface::GetSwapchainImageViews() {
  return m_SwapchainImageViews;
}
void VKSurface::Destroy(VkInstance instance, VkDevice device) {
  for(const auto& iv : m_SwapchainImageViews) {
    vkDestroyImageView(device, iv, nullptr);
  }

  for (const auto& fb : m_Framebuffers) {
    vkDestroyFramebuffer(device, fb, nullptr);
  }
  vkDestroySwapchainKHR(device, m_Swapchain, nullptr);
  vkDestroySurfaceKHR(instance, m_Surface, nullptr);
  vkDestroyRenderPass(device, m_PresentRenderPass, nullptr);
  SDL_DestroyWindow(m_Window);
}
VkSwapchainKHR VKSurface::GetSwapchain() {
  return m_Swapchain;
}
VkPresentModeKHR VKSurface::SelectPresentMode(const std::vector<VkPresentModeKHR> &modes) {
  VkPresentModeKHR presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;

  /*for (const auto& mode : modes) {
    if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
      presentMode = mode;
      break;
    }
  }*/

  return presentMode;
}
VkRenderPass &VKSurface::GetRenderPass() {
  return m_PresentRenderPass;
}
VkFramebuffer &VKSurface::GetFramebuffer(u32 index) {
  return m_Framebuffers[index];
}
