#pragma once

#include "../Utils/Debugger.h"
#include <vector>
#include <vulkan/vulkan.h>
#include <stdexcept>

namespace Magma {
class MRenderer {
public:
  MRenderer(VkDevice device, VkSwapchainKHR &swapchain, uint32_t &swapchainImageCount,
            const std::vector<VkImage> &swapchainImages,
            std::vector<VkImageView> &swapchainImagesViews,
            MDebugger &debugger);
  ~MRenderer();

  void createImageViews(VkFormat imageFormat) const;
  void destroyImageViews() const;

private:
  /*=============================================================================
    Handles and references
    =============================================================================*/
  VkDevice vkDevice{VK_NULL_HANDLE};
  VkSwapchainKHR *pSwapchain{nullptr};
  uint32_t *pSwapchainImageCount{nullptr};

  const std::vector<VkImage> &pSwapchainImages;
  std::vector<VkImageView> &pSwapchainImagesViews;

  MDebugger *pDebugger{nullptr};
};
};