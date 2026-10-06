#include "../../../include/Magma/Vulkan/Renderer.h"

using namespace Magma;

/*=============================================================================
  Class init and destruction
  =============================================================================*/
MRenderer::MRenderer(VkDevice device, VkSwapchainKHR &swapchain, uint32_t &swapchainImageCount,
                     const std::vector<VkImage> &swapchainImages,
                     std::vector<VkImageView> &swapchainImagesViews,
                     MDebugger &debugger)
    : vkDevice(device),
      pSwapchain(&swapchain),
      pSwapchainImageCount(&swapchainImageCount),
      pSwapchainImages(swapchainImages),
      pSwapchainImagesViews(swapchainImagesViews),
      pDebugger(&debugger) {
  pDebugger->print("Initializing renderer . . .");
}

MRenderer::~MRenderer() {
  pDebugger->print("Destroying renderer . . .");
  destroyImageViews();

  pSwapchain = nullptr;
  pSwapchainImageCount = nullptr;
  pDebugger = nullptr;
}

/*=============================================================================
  Rendering logic
  =============================================================================*/
void MRenderer::createImageViews(const VkFormat imageFormat) const {
  pDebugger->print("Creating swapchain image views . . .");
  pSwapchainImagesViews.resize(pSwapchainImages.size());

  for (size_t i = 0; i < pSwapchainImages.size(); i++) {
    VkImageViewCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    createInfo.image = pSwapchainImages[i];
    createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    createInfo.format = imageFormat;

    createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

    createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    createInfo.subresourceRange.baseMipLevel = 0;
    createInfo.subresourceRange.levelCount = 1;
    createInfo.subresourceRange.baseArrayLayer = 0;
    createInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(vkDevice, &createInfo, nullptr, &pSwapchainImagesViews[i]) != VK_SUCCESS) {
      throw std::runtime_error("Failed to create swapchain image views!");
    }
  }
  pDebugger->print("Created swapchain image views.");
}

void MRenderer::destroyImageViews() const {
  pDebugger->print("Destroying swapchain image views . . .");
  for (auto view : pSwapchainImagesViews) {
    if (view != VK_NULL_HANDLE) {
      vkDestroyImageView(vkDevice, view, nullptr);
    }
  }
  pSwapchainImagesViews.clear();
}