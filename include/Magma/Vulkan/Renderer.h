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
            const VkDevice &logicalDevice, const VkSurfaceKHR &surface,
            VkQueue graphicsQueue,VkQueue presentQueue);
  ~MRenderer();

  /*=============================================================================
    Rendering
    =============================================================================*/
  [[nodiscard]] bool canRenderFrame() const {return updateFrames;};
  [[nodiscard]] bool renderFrame();
  void drawFrame() const;
  [[nodiscard]] VkResult presentFrame() const;
  //Fence logic
  void createFence();
  void destroyFence() const;
  void waitForFence() const;
  void resetFence() const;

  /*=============================================================================
    Swapchain
    =============================================================================*/
  void createSwapchain();
  void destroySwapchain() const;
  void querySwapchainSupport();
  void setExtent(uint32_t width, uint32_t height);
  void recreateSwapchain();
  // Images
  void createImageViews();
  void destroyImageViews();

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

  /*=============================================================================
    Command buffer
    =============================================================================*/
  void createCommandBuffer();
  void recordCommandBuffer() const;

  /*=============================================================================
    Semaphores
    =============================================================================*/
  void createSemaphores();
  void destroySemaphores() const;


  /*=============================================================================
    Graphics pipeline
    =============================================================================*/
  void createGraphicsPipeline();
  void destroyGraphicsPipeline() const;

  /*=============================================================================
    Shaders
    =============================================================================*/
  [[nodiscard]] std::vector<char> readShaderFile(const std::string &fileName) const;
  void createShaderModule(const std::vector<char> &code,VkShaderModule &shaderModule) const;
  void destroyShaderModule() const;
private:
  /*=============================================================================
    Rendering
    =============================================================================*/
  uint32_t imageIndex{1};
  bool updateFrames{true};
  VkFence syncHostWithDeviceFence{VK_NULL_HANDLE};

  /*=============================================================================
     Swapchain
    =============================================================================*/
  VkSwapchainKHR Swapchain{VK_NULL_HANDLE};
  // Images
  unsigned int swapchainImageCount{2};
  std::vector<VkImage> swapchainImages;
  std::vector<VkImageView> swapchainImageViews;
  static void transitionImage(VkCommandBuffer cmd, VkImage image,
                                VkImageLayout oldLayout, VkImageLayout newLayout,
                                VkPipelineStageFlags2 srcStage, VkAccessFlags2 srcAccess,
                                VkPipelineStageFlags2 dstStage, VkAccessFlags2 dstAccess) ;

  /*=============================================================================
    Surface
    =============================================================================*/
  VkSurfaceKHR Surface{VK_NULL_HANDLE};
  std::optional<VkSurfaceFormatKHR> surfaceFormat{};
  std::vector<VkSurfaceFormatKHR> surfaceFormats;
  std::optional<VkPresentModeKHR> surfacePresentMode{};
  std::vector<VkPresentModeKHR> surfacePresentModes{};
  VkSurfaceCapabilitiesKHR surfaceCapabilities{};
  VkExtent2D extent{.width = 1, .height = 1};

  /*=============================================================================
    Command pool
    =============================================================================*/
  VkCommandPool commandPool{VK_NULL_HANDLE};


  VkRenderPass renderPass{VK_NULL_HANDLE};

  /*=============================================================================
    Command buffer
    =============================================================================*/
  VkCommandBuffer commandBuffer{VK_NULL_HANDLE};

  /*=============================================================================
    Semaphores
    =============================================================================*/
  VkSemaphore semaphoreImageAvailable{VK_NULL_HANDLE};
  VkSemaphore semaphoreRendererFinished{VK_NULL_HANDLE};

  /*=============================================================================
    Vulkan context
    =============================================================================*/
  VkPhysicalDevice physicalDevice{VK_NULL_HANDLE};
  VkDevice logicalDevice{VK_NULL_HANDLE};
  VkQueue graphicsQueue{VK_NULL_HANDLE};
  VkQueue presentQueue{VK_NULL_HANDLE};

  /*=============================================================================
    Graphics pipeline
    =============================================================================*/
  VkPipeline graphicsPipeline{VK_NULL_HANDLE};
  VkPipelineLayout pipelineLayout{VK_NULL_HANDLE};

  /*=============================================================================
    Shaders
    =============================================================================*/
  VkShaderModule vertShaderModule{VK_NULL_HANDLE};
  VkShaderModule fragShaderModule{VK_NULL_HANDLE};
  VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
  VkPipelineShaderStageCreateInfo fragShaderStageInfo{};

  /*=============================================================================
    Debugging
    =============================================================================*/
  MDebugger *pDebugger{nullptr};
};
}; // namespace Magma