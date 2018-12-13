#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include "../../../CommonTypes.h"
class VKDevice {
public:
  VKDevice();
  void SetupDevice(VkInstance instance, const std::vector<const char *> &requiredExtensions, VkSurfaceKHR surface);
  void DestroyDevice();
  VkPhysicalDevice GetPhysicalDevice();
  VkDevice GetDevice();
  VkQueue GetGraphicsQueue();
  VkQueue GetPresentQueue();
  VkCommandPool GetCommandPool();
  VkDescriptorPool GetDescriptorPool();
  VkPhysicalDeviceProperties GetDeviceProperties();
  std::vector<VkCommandBuffer> AllocateCommandBuffers(VkCommandBufferLevel level, u32 count);
  void FreeCommandBuffers(std::vector<VkCommandBuffer> buffers);
private:
  VkPhysicalDevice m_PhysDevice;
  VkDevice m_Device;
  VkPhysicalDeviceProperties m_PhysDeviceProperties;
  u32 m_GraphicsQueueFamily;
  u32 m_PresentQueueFamily;
  VkQueue m_GraphicsQueue;
  VkQueue m_PresentQueue;
  VkCommandPool m_CommandPool;
  VkDescriptorPool m_DescriptorPool;
};
