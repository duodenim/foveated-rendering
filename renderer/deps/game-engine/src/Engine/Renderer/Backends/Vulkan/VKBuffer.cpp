#include "VKBuffer.h"
VKBuffer::VKBuffer() {
  m_Buffer = VK_NULL_HANDLE;
  m_CachedMapPtr = nullptr;
  mSize = 0;
}
void VKBuffer::Setup(VkDeviceSize size,
                     VkBufferUsageFlags bufferUsage,
                     VmaMemoryUsage memoryUsage,
                     VmaAllocator allocator) {
  VkBufferCreateInfo createInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
  createInfo.usage = bufferUsage;
  createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  createInfo.size = size;

  VmaAllocationCreateInfo allocCreate = {};
  allocCreate.usage = memoryUsage;

  mSize = size;

  vmaCreateBuffer(allocator, &createInfo, &allocCreate, &m_Buffer, &m_Allocation, nullptr);
}
void VKBuffer::Destroy(VmaAllocator allocator) {
  vmaDestroyBuffer(allocator, m_Buffer, m_Allocation);
}
VkBuffer VKBuffer::GetBuffer() {
  return m_Buffer;
}
VmaAllocation VKBuffer::GetAllocation() {
  return m_Allocation;
}
void * VKBuffer::Map(VmaAllocator allocator) {
  if (m_CachedMapPtr == nullptr) {
    vmaMapMemory(allocator, m_Allocation, &m_CachedMapPtr);
  }
  return m_CachedMapPtr;
}
void VKBuffer::UnMap(VmaAllocator allocator) {
  if (m_CachedMapPtr != nullptr) {
    vmaUnmapMemory(allocator, m_Allocation);
  }
  m_CachedMapPtr = nullptr;
}
VkDescriptorBufferInfo VKBuffer::GetBufferInfo() {
  VkDescriptorBufferInfo ret = {};
  ret.buffer = m_Buffer;
  ret.offset = 0;
  ret.range = mSize;

  return ret;
}
