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
            VkExtent2D swapchainExtent,VkSurfaceFormatKHR surfaceFormat ,MDebugger &debugger);
  ~MRenderer();

  /*=============================================================================
    Swapchain
    =============================================================================*/
  // Images
  void createImageViews() const;
  void destroyImageViews() const;
  // Framebuffer
  void createFramebuffers();
  void destroyFramebuffers() const;
  // Command pool
  void createCommandPool(uint32_t graphicsQueueFamilyIndex);
  void destroyCommandPool() const;
  void createCommandBuffers();
  void createRenderPass();
  void destroyRenderPass() const;

private:
  /*=============================================================================
    Vulkan context
    =============================================================================*/
  VkDevice logicalDevice{VK_NULL_HANDLE};

  /*=============================================================================
     Swapchain
    =============================================================================*/
  VkSwapchainKHR *pSwapchain{nullptr};
  VkExtent2D *pSwapchainExtents{nullptr};
  //Surface
  VkSurfaceFormatKHR surfaceFormat{};
  // Images
  uint32_t *pSwapchainImageCount{nullptr};
  const std::vector<VkImage> &rSwapchainImages;
  std::vector<VkImageView> &rSwapchainImagesViews;
  // Framebuffer
  VkRenderPass renderPass{VK_NULL_HANDLE};
  std::vector<VkFramebuffer> swapchainFramebuffers;
  // Command pool
  VkCommandPool commandPool{VK_NULL_HANDLE};
 std::vector<VkCommandBuffer> commandBuffers;

  /*=============================================================================
    Debugging
    =============================================================================*/
  MDebugger *pDebugger{nullptr};
};
}; // namespace Magma