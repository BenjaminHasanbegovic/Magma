#pragma once

#include "../Utils/Debugger.h"
#include "../Utils/ErrorCodes.h"
#include <bits/fs_fwd.h>
#include <vulkan/vulkan.h>

namespace Magma {
class MSwapchain {
public:
  /*=============================================================================
     Class init and destruction
    =============================================================================*/
  explicit MSwapchain(MDebugger &debugger,
                      const VkPhysicalDevice &physicalDevice,
                      const VkDevice &logicalDevice,
                      const VkSurfaceKHR &surface);
  ~MSwapchain();

  /*=============================================================================
    Swapchain
    =============================================================================*/
  void querySupport();
  void create();
  void setExtent(uint32_t width, uint32_t height);

  //Getters
  [[nodiscard]] unsigned int &getImageCount() {return swapchainImageCount;};
  [[nodiscard]] std::vector<VkImage> &getImages() {return swapchainImages;};
  [[nodiscard]] VkSwapchainKHR &getSwapchain() {return Swapchain;};
  [[nodiscard]] std::vector<VkImageView> &getImageViews() {return swapchainImageViews;};

private:
  /*=============================================================================
    Debuging
    =============================================================================*/
  MDebugger *pDebugger;

  /*=============================================================================
    Swapchain
    =============================================================================*/
  // Info
  VkSurfaceCapabilitiesKHR surfaceCapabilities{};
  std::vector<VkSurfaceFormatKHR> surfaceFormats;
  std::vector<VkPresentModeKHR> presentModes;
  VkSwapchainKHR Swapchain{VK_NULL_HANDLE};
  unsigned int swapchainImageCount{2};
  std::vector<VkImage> swapchainImages;
  std::vector<VkImageView> swapchainImageViews;

  [[nodiscard]] VkSurfaceFormatKHR selectVkFormat() const;
  [[nodiscard]] VkPresentModeKHR selectVkPresentMode() const;

  /*=============================================================================
    Vulkan info
    =============================================================================*/
  VkSurfaceKHR pSurface{VK_NULL_HANDLE};
  VkPhysicalDevice pPhysicalDevice{VK_NULL_HANDLE};
  VkDevice pLogicalDevice{VK_NULL_HANDLE};
};
}; // namespace Magma