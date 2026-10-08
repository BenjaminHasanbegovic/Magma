#include "../../../include/Magma/Vulkan/Renderer.h"

using namespace Magma;

/*=============================================================================
  Class init and destruction
  =============================================================================*/
MRenderer::MRenderer(VkDevice logicalDevice, VkSwapchainKHR &swapchain,
                     uint32_t &swapchainImageCount,
                     const std::vector<VkImage> &swapchainImages,
                     std::vector<VkImageView> &swapchainImagesViews,
                     VkExtent2D swapchainExtent,
                     const VkSurfaceFormatKHR surfaceFormat,MDebugger &debugger)
    :logicalDevice(logicalDevice) ,pSwapchain(&swapchain),surfaceFormat(surfaceFormat) ,pSwapchainImageCount(&swapchainImageCount),
      rSwapchainImages(swapchainImages),
      rSwapchainImagesViews(swapchainImagesViews), pDebugger(&debugger) {


  pSwapchainExtents = &swapchainExtent;

  pDebugger->print("Initializing renderer . . .");
};

MRenderer::~MRenderer() {
  pDebugger->print("Destroying renderer . . .");

  destroyCommandPool();
  destroyFramebuffers();
  destroyImageViews();

  pSwapchain = nullptr;
  pSwapchainImageCount = nullptr;
  pDebugger = nullptr;
  pSwapchainExtents = nullptr;
};

/*=============================================================================
  Swapchain
  =============================================================================*/
void MRenderer::createImageViews() const {
  pDebugger->print("Creating swapchain image views . . .");
  rSwapchainImagesViews.resize(rSwapchainImages.size());

  for (size_t i = 0; i < rSwapchainImages.size(); i++) {
    VkImageViewCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    createInfo.image = rSwapchainImages[i];
    createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    createInfo.format = surfaceFormat.format;

    createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

    createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    createInfo.subresourceRange.baseMipLevel = 0;
    createInfo.subresourceRange.levelCount = 1;
    createInfo.subresourceRange.baseArrayLayer = 0;
    createInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(logicalDevice, &createInfo, nullptr,
                          &rSwapchainImagesViews[i]) != VK_SUCCESS) {
      throw std::runtime_error(
          "Failed to create swapchain image views!" +
          errorCodeToString(ErrorCode::IMAGE_VIEWS_CREATION_FAILED));
    };
  };
  pDebugger->print("Created swapchain image views.");
};

void MRenderer::destroyImageViews() const {
  pDebugger->print("Destroying swapchain image views . . .");
  for (const auto view : rSwapchainImagesViews) {
    if (view != VK_NULL_HANDLE) {
      vkDestroyImageView(logicalDevice, view, nullptr);
    };
  };
  rSwapchainImagesViews.clear();
};
//Framebuffer
void MRenderer::createFramebuffers() {
  pDebugger->print("Creating framebuffers . . .");

  swapchainFramebuffers.resize(rSwapchainImagesViews.size());

  for (size_t i = 0; i < rSwapchainImagesViews.size(); i++) {
    const VkImageView attachments[] = {
        rSwapchainImagesViews[i],
    };

    VkFramebufferCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    createInfo.renderPass = renderPass;
    createInfo.attachmentCount = 1;
    createInfo.pAttachments = attachments;
    createInfo.width = pSwapchainExtents->width;
    createInfo.height = pSwapchainExtents->height;
    createInfo.layers = 1;

    if (vkCreateFramebuffer(logicalDevice, &createInfo, nullptr,
                            &swapchainFramebuffers[i]) != VK_SUCCESS) {
      throw std::runtime_error("Failed to create framebuffers: " + errorCodeToString(ErrorCode::FRAMEBUFFER_CREATION_FAILED));
    };
  };

  pDebugger->print("Created framebuffers.");
};

void MRenderer::destroyFramebuffers() const {
  pDebugger->print("Destroying framebuffers . . .");

  for (const auto framebuffer : swapchainFramebuffers) {
    pDebugger->print("Destroying framebuffer . . .");
    vkDestroyFramebuffer(logicalDevice, framebuffer, nullptr);
  };
};
// Command pool
void MRenderer::createCommandPool(const uint32_t graphicsQueueFamilyIndex) {
  pDebugger->print("Creating command pool, queue family index:, " + std::to_string(graphicsQueueFamilyIndex) + " . . .");

  VkCommandPoolCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  createInfo.queueFamilyIndex = graphicsQueueFamilyIndex;
  createInfo.flags = 0;

  if (vkCreateCommandPool(logicalDevice, &createInfo, nullptr,&commandPool) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create command pool!");
  };

  pDebugger->print("Created command pool.");
};

void MRenderer::destroyCommandPool() const {
  pDebugger->print("Destroying command pool . . .");

  vkDestroyCommandPool(logicalDevice, commandPool, nullptr);

  pDebugger->print("Destroyed command pool.");
};

void MRenderer::createCommandBuffers() {
  pDebugger->print("Creating command buffers . . .");

  commandBuffers.resize(rSwapchainImagesViews.size());

  VkCommandBufferAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.commandPool = commandPool;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

  if (vkAllocateCommandBuffers(logicalDevice, &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
    throw std::runtime_error("Failed to allocate command buffers!");
  };

  pDebugger->print("Created command buffers .");
};