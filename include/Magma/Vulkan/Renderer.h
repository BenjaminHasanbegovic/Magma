#pragma once

#include "../Utils/Debugger.h"
#include <vector>
#include <vulkan/vulkan.h>
#include <filesystem>
#include <fstream>

namespace Magma {
class MRenderer {
public:
  /*=============================================================================
    Class init and destruction
    =============================================================================*/
  MRenderer(MDebugger &debugger, const VkPhysicalDevice &physicalDevice,
            const VkDevice &logicalDevice, const VkSurfaceKHR &surface);
  ~MRenderer();

  /*=============================================================================
    Swapchain
    =============================================================================*/
  void createSwapchain();
  void destroySwapchain() const;
  void querySwapchainSupport();
  void setExtent(uint32_t width, uint32_t height);
  // Images
  void createImageViews();
  void destroyImageViews();

  /*=============================================================================
    Framebuffer
    =============================================================================*/
  void createFramebuffers();
  void destroyFramebuffers() const;

  /*=============================================================================
    Surface
    =============================================================================*/
   void selectSurfaceFormat();
   void selectPresentMode();

  /*=============================================================================
    Command pool
    =============================================================================*/
  void createCommandPool(uint32_t graphicsQueueFamilyIndex);
  void destroyCommandPool() const;
  void createCommandBuffers();
  void createRenderPass();
  void destroyRenderPass() const;

  /*=============================================================================
    Graphics pipeline
    =============================================================================*/
  void createGraphicsPipeline();
  void destroyGraphicsPipeline() const;
  std::vector<char> readShaderFile(const std::string &fileName) const;
  void createShaderModule(const std::vector<char> &code,VkShaderModule &shaderModule) const;
private:
  /*=============================================================================
     Swapchain
    =============================================================================*/
  VkSwapchainKHR Swapchain{VK_NULL_HANDLE};
  std::vector<VkFramebuffer> swapchainFramebuffers;
  // Images
  unsigned int swapchainImageCount{2};
  std::vector<VkImage> swapchainImages;
  std::vector<VkImageView> swapchainImageViews;

  /*=============================================================================
    Surface
    =============================================================================*/
  VkSurfaceKHR Surface{VK_NULL_HANDLE};
  VkSurfaceFormatKHR surfaceFormat{};
  std::vector<VkSurfaceFormatKHR> surfaceFormats;
  VkPresentModeKHR surfacePresentMode;
  std::vector<VkPresentModeKHR> surfacePresentModes;
  VkSurfaceCapabilitiesKHR surfaceCapabilities{};
  VkExtent2D extent{.width = 1, .height = 1};

  /*=============================================================================
    Command pool
    =============================================================================*/
  VkCommandPool commandPool{VK_NULL_HANDLE};
  std::vector<VkCommandBuffer> commandBuffers;
  VkRenderPass renderPass{VK_NULL_HANDLE};

  /*=============================================================================
    Vulkan info
    =============================================================================*/
  VkPhysicalDevice physicalDevice{VK_NULL_HANDLE};
  VkDevice logicalDevice{VK_NULL_HANDLE};

  /*=============================================================================
    Graphics pipeline
    =============================================================================*/
  VkPipeline graphicsPipeline{VK_NULL_HANDLE};
  VkPipelineLayout pipelineLayout{VK_NULL_HANDLE};

  /*=============================================================================
    Debugging
    =============================================================================*/
  MDebugger *pDebugger{nullptr};
};
}; // namespace Magma