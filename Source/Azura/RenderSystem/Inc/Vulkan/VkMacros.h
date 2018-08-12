#pragma once

#include <stdexcept>
#include <vulkan/vulkan_core.h>

namespace Azura {
namespace Vulkan {
#define VERIFY_VK_OP(res, message)        \
  if((res) != VK_SUCCESS) {               \
    throw std::runtime_error(#message);   \
  }

#define VERIFY_TRUE(res, message)         \
  if((res) != true) {                     \
    throw std::runtime_error(#message);   \
  }

#define VERIFY_OPT(res, message)         \
  if((res)) {                     \
    throw std::runtime_error(#message);   \
  }

} // namespace Vulkan
} // namespace Azura