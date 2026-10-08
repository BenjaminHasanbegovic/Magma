#pragma once

#include "../Utils/Debugger.h"
#include "../Utils/ErrorCodes.h"
#include <vector>
#include <vulkan/vulkan.h>

namespace Magma {
class MRenderer {
public:
  /*=============================================================================
    Class init and destruction
    =============================================================================*/
  MRenderer(VkDevice logicalDevice, VkSwapchainKHR &swapchain,
            uint32_t &swapchainImageCount,
            const std::vector<VkImage> &swapchainImages,
            std::vector<VkImageView> &swapchainImagesViews,
            VkExtent2D swapchainExtent, MDebugger &debugger);
  ~MRenderer();

  /*=============================================================================
    Swapchain
    =============================================================================*/
  // Images
  void createImageViews(VkFormat imageFormat) const;
  void destroyImageViews() const;
  // Framebuffer
  void createFramebuffers();
  void destroyFramebuffers() const;

private:
  /*=============================================================================
    Vulkan context
    =============================================================================*/
  VkDevice *pLogicalDevice{VK_NULL_HANDLE};

  /*=============================================================================
     Swapchain
    =============================================================================*/
  VkSwapchainKHR *pSwapchain{nullptr};
  VkExtent2D *pSwapchainExtents{nullptr};
  // Images
  uint32_t *pSwapchainImageCount{nullptr};
  const std::vector<VkImage> &rSwapchainImages;
  std::vector<VkImageView> &rSwapchainImagesViews;
  // Framebuffer
  VkRenderPass renderPass{VK_NULL_HANDLE};
  std::vector<VkFramebuffer> swapchainFramebuffers;

  /*=============================================================================
    Debugging
    =============================================================================*/
  MDebugger *pDebugger{nullptr};
};
}; // namespace Magma