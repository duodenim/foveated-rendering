#pragma once

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
class VKBuffer {
public:
  VKBuffer();
  void Setup(VkDeviceSize size, VkBufferUsageFlags bufferUsage, VmaMemoryUsage memoryUsage, VmaAllocator allocator);
  void Destroy(VmaAllocator allocator);
  VkBuffer GetBuffer();
  VmaAllocation GetAllocation();
  void* Map(VmaAllocator allocator);
  void UnMap(VmaAllocator allocator);
  VkDescriptorBufferInfo GetBufferInfo();
private:
  VkBuffer m_Buffer;
  VmaAllocation m_Allocation;
  VkDeviceSize mSize;
  void* m_CachedMapPtr;
};