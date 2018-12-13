#pragma once
#include <vector>
#include <string>
#include <SDL_video.h>
#include <vulkan/vulkan.h>
#include "../../../CommonTypes.h"

class VKSurface {
public:
  VKSurface();
  void CreateWindow(const std::string& windowName, const int width, const int height);
  void CreateSurface(VkInstance instance);
  void CreateSwapchain(VkPhysicalDevice physicalDevice, VkDevice device);
  void Destroy(VkInstance instance, VkDevice device);
  VkSurfaceKHR GetSurface();
  VkSurfaceCapabilitiesKHR GetCapabilities();
  VkSurfaceFormatKHR GetDefaultFormat();
  VkExtent2D GetSwapchainExtent();
  std::vector<const char *> GetInstanceExtensions();
  u32 GetSwapchainImageCount();
  std::vector<VkImageView> GetSwapchainImageViews();
  VkSwapchainKHR GetSwapchain();
  VkRenderPass& GetRenderPass();
  VkFramebuffer& GetFramebuffer(u32 index);
private:

  VkSurfaceFormatKHR SelectSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &formats);
  VkPresentModeKHR SelectPresentMode(const std::vector<VkPresentModeKHR> &modes);

  SDL_Window* m_Window;
  VkSurfaceKHR m_Surface;
  VkSwapchainKHR m_Swapchain;
  std::vector<VkImage> m_SwapchainImages;
  std::vector<VkImageView> m_SwapchainImageViews;
  VkRenderPass m_PresentRenderPass;
  std::vector<VkFramebuffer> m_Framebuffers;
  VkSurfaceCapabilitiesKHR m_SurfaceCapabilities;
  std::vector<VkSurfaceFormatKHR> m_SurfaceFormats;
  VkSurfaceFormatKHR m_DefaultSurfaceFormat;
  VkExtent2D m_SwapchainExtent;
  std::vector<VkPresentModeKHR> m_SurfacePresentModes;
};
