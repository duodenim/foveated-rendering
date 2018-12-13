#include "VKDevice.h"
#include "../../../Log.h"
#include <set>
#include "VKError.h"

const u32 INVALID_QUEUE_INDEX = 0xffffffff;

const float QUEUE_PRIORITY = 1.0f;

const u32 MAX_ALLOCATED_UBOS = 16;
const u32 MAX_ALLOCATED_IMAGES = 2048;
const u32 MAX_ALLOCATED_SETS = 2048;

VKDevice::VKDevice() {
  m_PhysDevice = VK_NULL_HANDLE;
  m_Device = VK_NULL_HANDLE;
  m_GraphicsQueueFamily = INVALID_QUEUE_INDEX;
  m_PresentQueueFamily = INVALID_QUEUE_INDEX;
  m_GraphicsQueue = VK_NULL_HANDLE;
  m_PresentQueue = VK_NULL_HANDLE;
  m_CommandPool = VK_NULL_HANDLE;
  m_DescriptorPool = VK_NULL_HANDLE;
  m_PhysDeviceProperties = {};
}
void VKDevice::SetupDevice(VkInstance instance, const std::vector<const char *> &requiredExtensions, VkSurfaceKHR surface) {
  //Get all devices
  u32 physDeviceCount = 0;
  vkEnumeratePhysicalDevices(instance, &physDeviceCount, nullptr);

  std::vector<VkPhysicalDevice> physDevices(physDeviceCount);

  vkEnumeratePhysicalDevices(instance, &physDeviceCount, physDevices.data());

  if (physDevices.empty()) {
    Log::LogFatal("[VKBackend] Could not find any devices");
  }

  //Log device names
  Log::LogInfo("[VKBackend] Available devices:");
  for (const auto& device : physDevices) {
    VkPhysicalDeviceProperties devProps;
    vkGetPhysicalDeviceProperties(device, &devProps);
    Log::LogInfo("[VKBackend] " + std::string(devProps.deviceName));
  }

  //Pick the first available device
  m_PhysDevice = physDevices[0];

  //Log some info about the device
  vkGetPhysicalDeviceProperties(m_PhysDevice, &m_PhysDeviceProperties);

  Log::LogInfo("[VKBackend] Using Device: " + std::string(m_PhysDeviceProperties.deviceName));
  Log::LogInfo("[VKBackend] " + std::to_string(m_PhysDeviceProperties.limits.maxMemoryAllocationCount) + " Memory Allocations are possible");
  Log::LogInfo("[VKBackend] " + std::to_string(m_PhysDeviceProperties.limits.maxPushConstantsSize) + " bytes is the max push constant size");

  //Check for required push constant size
  if (m_PhysDeviceProperties.limits.maxPushConstantsSize < sizeof(Mat4)) {
    Log::LogFatal("[VKBackend] Device Push constant size is not high enough");
    Log::LogFatal("[VKBackend] " + std::to_string(sizeof(Mat4)) + " bytes are necessary");
    exit(1);
  }

  //Print device extensions
  u32 extCount = 0;
  vkEnumerateDeviceExtensionProperties(m_PhysDevice, nullptr, &extCount, nullptr);
  std::vector<VkExtensionProperties> devExtensions(extCount);
  vkEnumerateDeviceExtensionProperties(m_PhysDevice, nullptr, &extCount, devExtensions.data());

  Log::LogInfo("[VKBackend] Available device extensions:");

  for (const auto &ext : devExtensions) {
    Log::LogInfo("[VKBackend] " + std::string(ext.extensionName));
  }

  //Look for usable queue families
  u32 queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(m_PhysDevice, &queueFamilyCount, nullptr);
  std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(m_PhysDevice, &queueFamilyCount, queueFamilies.data());

  //Get usable graphics queue family
  for (u32 i = 0; i < queueFamilies.size(); i++) {
    if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT && queueFamilies[i].queueCount > 0) {
      m_GraphicsQueueFamily = i;
    }
  }

  //Get usable present queue family
  for (u32 i = 0; i < queueFamilies.size(); i++) {
    VkBool32 presentSupport;
    vkGetPhysicalDeviceSurfaceSupportKHR(m_PhysDevice, i, surface, &presentSupport);

    if (queueFamilies[i].queueCount > 0 && presentSupport == VK_TRUE) {
      m_PresentQueueFamily = i;
    }
  }

  if (m_GraphicsQueueFamily == INVALID_QUEUE_INDEX || m_PresentQueueFamily == INVALID_QUEUE_INDEX) {
    Log::LogFatal("[VKBackend] No present/graphics queue available");
  }

  std::set<u32> uniqueQueueFamilies;
  uniqueQueueFamilies.insert(m_PresentQueueFamily);
  uniqueQueueFamilies.insert(m_GraphicsQueueFamily);

  std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

  for (const auto& uniqueQueueFamily : uniqueQueueFamilies) {
    VkDeviceQueueCreateInfo queueCreateInfo = {VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
    queueCreateInfo.queueFamilyIndex = uniqueQueueFamily;
    queueCreateInfo.pQueuePriorities = &QUEUE_PRIORITY;
    queueCreateInfo.queueCount = 1;
    queueCreateInfos.push_back(queueCreateInfo);
  }
  //Create logical device
  VkDeviceCreateInfo deviceCreateInfo = {VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
  deviceCreateInfo.pEnabledFeatures = nullptr;
  deviceCreateInfo.enabledExtensionCount = (u32)requiredExtensions.size();
  deviceCreateInfo.ppEnabledExtensionNames = requiredExtensions.data();
  deviceCreateInfo.queueCreateInfoCount = (u32)queueCreateInfos.size();
  deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
  vkCreateDevice(m_PhysDevice, &deviceCreateInfo, nullptr, &m_Device);

  //Get queue handles
  vkGetDeviceQueue(m_Device, m_GraphicsQueueFamily, 0, &m_GraphicsQueue);
  vkGetDeviceQueue(m_Device, m_PresentQueueFamily, 0, &m_PresentQueue);

  //Create command pool
  VkCommandPoolCreateInfo poolCreateInfo = {VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
  poolCreateInfo.queueFamilyIndex = m_GraphicsQueueFamily;
  poolCreateInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  vkCreateCommandPool(m_Device, &poolCreateInfo, nullptr, &m_CommandPool);

  //Create descriptor pool
  VkDescriptorPoolSize poolSizeUBO;
  poolSizeUBO.descriptorCount = MAX_ALLOCATED_UBOS;
  poolSizeUBO.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

  VkDescriptorPoolSize poolSizeTex;
  poolSizeTex.descriptorCount = MAX_ALLOCATED_IMAGES;
  poolSizeTex.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

  VkDescriptorPoolSize poolSizes[] = { poolSizeUBO, poolSizeTex };
  VkDescriptorPoolCreateInfo poolInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
  poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
  poolInfo.poolSizeCount = 2;
  poolInfo.pPoolSizes = poolSizes;
  poolInfo.maxSets = MAX_ALLOCATED_SETS;

  VKError::CheckResult(vkCreateDescriptorPool(m_Device, &poolInfo, nullptr, &m_DescriptorPool), "Could not create descriptor pool");
}
void VKDevice::DestroyDevice() {
  vkDestroyDescriptorPool(m_Device, m_DescriptorPool, nullptr);
  vkDestroyCommandPool(m_Device, m_CommandPool, nullptr);
  vkDestroyDevice(m_Device, nullptr);
}
VkPhysicalDevice VKDevice::GetPhysicalDevice() {
  return m_PhysDevice;
}
VkDevice VKDevice::GetDevice() {
  return m_Device;
}
VkQueue VKDevice::GetGraphicsQueue() {
  return m_GraphicsQueue;
}
VkQueue VKDevice::GetPresentQueue() {
  return m_PresentQueue;
}
VkCommandPool VKDevice::GetCommandPool() {
  return m_CommandPool;
}
VkPhysicalDeviceProperties VKDevice::GetDeviceProperties() {
  return m_PhysDeviceProperties;
}
VkDescriptorPool VKDevice::GetDescriptorPool() {
  return m_DescriptorPool;
}
std::vector<VkCommandBuffer> VKDevice::AllocateCommandBuffers(VkCommandBufferLevel level, u32 count) {
  VkCommandBufferAllocateInfo allocateInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
  allocateInfo.commandBufferCount = count;
  allocateInfo.level = level;
  allocateInfo.commandPool = m_CommandPool;

  std::vector<VkCommandBuffer> return_buffers(count);
  VKError::CheckResult(vkAllocateCommandBuffers(m_Device, &allocateInfo, return_buffers.data()), "Could not allocate command buffers");

  return return_buffers;
}
void VKDevice::FreeCommandBuffers(std::vector<VkCommandBuffer> buffers) {
  vkFreeCommandBuffers(m_Device, m_CommandPool, buffers.size(), buffers.data());
}

