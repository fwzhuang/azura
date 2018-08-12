#pragma once

#include "Generic/Shader.h"
#include <vulkan/vulkan_core.h>
#include <map>
#include <vector>

namespace Azura {
namespace Vulkan {

class VkShader : public Shader {
public:
  VkShader(VkDevice device, const String& fileName);

  VkPipelineShaderStageCreateInfo GetShaderStageInfo() const;

  const std::vector<VkVertexInputAttributeDescription>& GetInputAttributeDescription() const;

  void AddVertexAttribute(VkFormat format, U32 binding);

private:
  struct BindingInfo {
    U32 m_offset{0};
    U32 m_location{0};
  };

  std::map<U32, BindingInfo> m_bindingMap;
  VkShaderModule m_module;
  std::vector<VkVertexInputAttributeDescription> m_attributes;
};


} // namespace Vulkan
} // namespace Azura