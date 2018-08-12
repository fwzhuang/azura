#include "Vulkan/VkScopedBuffer.h"
#include "Vulkan/VkMacros.h"
#include "Vulkan/VkCore.h"


namespace Azura {
namespace Vulkan {

VkScopedBuffer::VkScopedBuffer(VkDevice device,
                               const VkBufferUsageFlags usage,
                               const U32 bufferSize,
                               const VkMemoryPropertyFlags memoryProperties,
                               const VkPhysicalDeviceMemoryProperties& phyDeviceMemoryProperties)
  : m_device(device) {
  VkBufferCreateInfo bufferInfo = {};
  bufferInfo.sType              = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.size               = bufferSize;
  bufferInfo.usage              = usage;
  bufferInfo.sharingMode        = VK_SHARING_MODE_EXCLUSIVE;

  // TODO(vasumahesh1): Need an encapsulation for Sharing Mode

  VERIFY_VK_OP(vkCreateBuffer(device, &bufferInfo, nullptr, &m_buffer), "Failed to create buffer");

  VkMemoryRequirements memRequirements;
  vkGetBufferMemoryRequirements(device, m_buffer, &memRequirements);

  VkMemoryAllocateInfo allocInfo = {};
  allocInfo.sType                = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize       = memRequirements.size;
  allocInfo.memoryTypeIndex      =
    VkCore::FindMemoryType(memRequirements.memoryTypeBits, memoryProperties, phyDeviceMemoryProperties);

  VERIFY_VK_OP(vkAllocateMemory(device, &allocInfo, nullptr, &m_memory), "Failed to allocate buffer memory")

  vkBindBufferMemory(device, m_buffer, m_memory, 0);
}

void* VkScopedBuffer::MapMemory(VkDeviceSize size, VkDeviceSize offset) const {
  void* data;
  vkMapMemory(m_device, m_memory, offset, size, 0, &data);
  return data;
}

void VkScopedBuffer::UnmapMemory() const {
  vkUnmapMemory(m_device, m_memory);
}

VkBuffer VkScopedBuffer::Real() const {
  return m_buffer;
}

VkDeviceMemory VkScopedBuffer::Memory() const {
  return m_memory;
}

VkDescriptorBufferInfo VkScopedBuffer::GetDescriptorInfo(U32 offset, U32 range) const {
  VkDescriptorBufferInfo bufferInfo;
  bufferInfo.buffer = Real();
  bufferInfo.offset = offset;
  bufferInfo.range = range;

  return bufferInfo;
}
} // namespace Vulkan
} // namespace Azura