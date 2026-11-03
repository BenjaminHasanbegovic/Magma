#include "../../../include/Magma/Vulkan/Renderer.h"

using namespace Magma;

/*=============================================================================
  Class init and destruction
  =============================================================================*/
MRenderer::MRenderer(MDebugger &debugger,
                     const VkPhysicalDevice &physicalDevice,
                     const VkDevice &logicalDevice, const VkSurfaceKHR &surface,
                     VkQueue graphicsQueue, VkQueue presentQueue)
    : Surface(surface), physicalDevice(physicalDevice),
      logicalDevice(logicalDevice), graphicsQueue(graphicsQueue),
      presentQueue(presentQueue), pDebugger(&debugger) {
  pDebugger->print("Constructed renderer .");
};

MRenderer::~MRenderer() {
  pDebugger->print("Destroying renderer . . .");

  vkDeviceWaitIdle(logicalDevice);
  destroyFence();
  destroySemaphores();
  destroyGraphicsPipeline();
  destroyCommandPool();
  destroyImageViews();
  destroySwapchain();
  destroyShaderModule();
  pDebugger = nullptr;
};

 /*=============================================================================
   Rendering
   =============================================================================*/
bool MRenderer::renderFrame(){
  waitForFence();

  const VkResult acquire = vkAcquireNextImageKHR(logicalDevice, Swapchain,UINT64_MAX,semaphoreImageAvailable,syncHostWithDeviceFence,&imageIndex);

  if (acquire == VK_ERROR_OUT_OF_DATE_KHR) return true;
  if (acquire != VK_SUCCESS && acquire != VK_SUBOPTIMAL_KHR)
    throw std::runtime_error("Failed to acquire swapchain image!");

  resetFence();
  recordCommandBuffer();
  drawFrame();

  const VkResult present = presentFrame();
  if (present == VK_ERROR_OUT_OF_DATE_KHR || present == VK_SUBOPTIMAL_KHR) return true;

  if (present != VK_SUCCESS) throw std::runtime_error("Failed to present swapchain image!");
  return false;
};

void MRenderer::drawFrame() const {
  const VkSemaphore waitSemaphores[] = { semaphoreImageAvailable };
  const VkSemaphore signalSemaphores[] = { semaphoreRendererFinished };
  constexpr VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.waitSemaphoreCount = 1;
  submitInfo.pWaitSemaphores = waitSemaphores;
  submitInfo.pWaitDstStageMask = waitStages;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &commandBuffer;
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores = signalSemaphores;

  // Error handling
  if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, syncHostWithDeviceFence) != VK_SUCCESS) throw std::runtime_error("Failed to submit the queue!");
};

VkResult MRenderer::presentFrame() const {
  const VkSemaphore signalSemaphores[]{ semaphoreRendererFinished };
  const VkSwapchainKHR swapchains[]{Swapchain};

  VkPresentInfoKHR presentInfo{};
  presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores = signalSemaphores;
  presentInfo.swapchainCount = 1;
  presentInfo.pSwapchains = swapchains;
  presentInfo.pImageIndices = &imageIndex;
  presentInfo.pResults = nullptr;

  return vkQueuePresentKHR(presentQueue, &presentInfo);
};

void MRenderer::createFence() {
  pDebugger->print("Creating fence . . .");

VkFenceCreateInfo createInfo{};

  createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  createInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

  //Error handling
  if (vkCreateFence(logicalDevice, &createInfo, nullptr, &syncHostWithDeviceFence) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create Vulkan Fence!");
  };
};

void MRenderer::destroyFence() const {
  pDebugger->print("Destroying fence . . .");

  vkDestroyFence(logicalDevice, syncHostWithDeviceFence, nullptr);
};

void MRenderer::waitForFence() const {
  if (vkWaitForFences(logicalDevice, 1, &syncHostWithDeviceFence, VK_TRUE, UINT64_MAX) != VK_SUCCESS)
    throw std::runtime_error("Failed to wait for fence!");
};

void MRenderer::resetFence() const {
  if (vkResetFences(logicalDevice, 1, &syncHostWithDeviceFence) != VK_SUCCESS) throw std::runtime_error("Failed to reset fence!");
};

/*=============================================================================
  Swapchain
  =============================================================================*/
void MRenderer::createSwapchain() {
pDebugger->print("Creating swapchain . . .");

  if (!surfaceFormat.has_value()) selectSurfaceFormat();

  if (!surfacePresentMode.has_value()) selectPresentMode();

  swapchainImageCount = surfaceCapabilities.minImageCount + 1;
  if (surfaceCapabilities.maxImageCount > 0 &&
      swapchainImageCount > surfaceCapabilities.maxImageCount) {
    swapchainImageCount = surfaceCapabilities.maxImageCount;
  };

  VkSwapchainKHR oldSwapchain = Swapchain;
  VkSwapchainCreateInfoKHR createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  createInfo.oldSwapchain = oldSwapchain;
  createInfo.surface = Surface;
  createInfo.minImageCount = swapchainImageCount;
  createInfo.imageFormat = surfaceFormat.value().format;
  createInfo.imageColorSpace =  surfaceFormat.value().colorSpace;
  createInfo.imageArrayLayers = 1;
  createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  createInfo.preTransform = surfaceCapabilities.currentTransform;
  createInfo.imageExtent = surfaceCapabilities.currentExtent;
  createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  createInfo.presentMode = surfacePresentMode.value();
  createInfo.compositeAlpha =
      VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; // Reminder: Move to window handler logic
  createInfo.clipped = VK_TRUE;
  createInfo.oldSwapchain = VK_NULL_HANDLE;

  // Creating the swapchain
  if (vkCreateSwapchainKHR(logicalDevice, &createInfo, nullptr, &Swapchain) !=
      VK_SUCCESS) {
    throw std::runtime_error("Failed to create Vulkan Swapchain!");
  };

  if (oldSwapchain != VK_NULL_HANDLE) vkDestroySwapchainKHR(logicalDevice, oldSwapchain, nullptr);

  // Retrieve the handle of swapchain image
  vkGetSwapchainImagesKHR(logicalDevice, Swapchain, &swapchainImageCount,
                          nullptr);
  swapchainImages.resize(swapchainImageCount);
  vkGetSwapchainImagesKHR(logicalDevice, Swapchain, &swapchainImageCount,
                          swapchainImages.data());


  pDebugger->print("Created swapchain.");
};

void MRenderer::destroySwapchain() const {
  pDebugger->print("Destroying swapchain.");

  vkDestroySwapchainKHR(logicalDevice, Swapchain, nullptr);
};

void MRenderer::recreateSwapchain() {
  vkDeviceWaitIdle(logicalDevice);
  destroyImageViews();
  createSwapchain();
  createImageViews();
};

void MRenderer::querySwapchainSupport() {
  pDebugger->print("Querying swapchain support . . .");

  auto getSurfaceCapabilities = [this]() {
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, Surface,
                                              &surfaceCapabilities);
    pDebugger->print("Fetched surface capabilities.");
  };

  auto getSurfaceFormats = [this]() {
    uint32_t surfaceFormatCount{0};

    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, Surface,
                                         &surfaceFormatCount, nullptr);
    if (surfaceFormatCount == 0) {
      throw std::runtime_error("No surface format found.");
    }

    surfaceFormats.resize(surfaceFormatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(
        physicalDevice, Surface, &surfaceFormatCount, surfaceFormats.data());

    pDebugger->print("Fetched surface formats.");
  };

  auto getSurfacePresentMode = [this]() {
    uint32_t presentModeCount = 0;
    VkResult result = vkGetPhysicalDeviceSurfacePresentModesKHR(
        physicalDevice, Surface, &presentModeCount, nullptr);

    if (result != VK_SUCCESS) {
      throw std::runtime_error("Failed to query surface present mode count.");
    };

    if (presentModeCount == 0) {
      throw std::runtime_error("No surface present modes available.");
    };

    surfacePresentModes.resize(presentModeCount);
    result = vkGetPhysicalDeviceSurfacePresentModesKHR(
        physicalDevice, Surface, &presentModeCount, surfacePresentModes.data());
    if (result == VK_SUCCESS) {
      pDebugger->print("Fetched surface present modes.");
    } else {
      throw std::runtime_error("Failed to retrieve surface present modes.");
    };
  };

  getSurfaceCapabilities();
  getSurfaceFormats();
  getSurfacePresentMode();

  pDebugger->print("Swapchain supported.");
};

void MRenderer::setExtent(const uint32_t width, const uint32_t height) {
  pDebugger->print("Setting surface extent . . .");

  surfaceCapabilities.currentExtent.width = width;
  surfaceCapabilities.currentExtent.height = height;

  extent = {.width = width, .height = height};

  pDebugger->print("Set the surface extent.");
};
// Images
void MRenderer::createImageViews() {
  pDebugger->print("Creating swapchain image views . . .");
  swapchainImageViews.resize(swapchainImages.size());

  for (size_t i = 0; i < swapchainImages.size(); i++) {
    VkImageViewCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    createInfo.image = swapchainImages[i];
    createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    createInfo.format = surfaceFormat.value().format;

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
                          &swapchainImageViews[i]) != VK_SUCCESS) {
      throw std::runtime_error(
          "Failed to create swapchain image views!");
    };
  };
  pDebugger->print("Created swapchain image views.");
};

void MRenderer::destroyImageViews() {
  pDebugger->print("Destroying swapchain image views . . .");
  for (const auto view : swapchainImageViews) {
    if (view != VK_NULL_HANDLE) {
      vkDestroyImageView(logicalDevice, view, nullptr);
    };
  };
  swapchainImageViews.clear();
};

void MRenderer::transitionImage(VkCommandBuffer cmd, VkImage image,
                                VkImageLayout oldLayout, VkImageLayout newLayout,
                                VkPipelineStageFlags2 srcStage, VkAccessFlags2 srcAccess,
                                VkPipelineStageFlags2 dstStage, VkAccessFlags2 dstAccess) {
  VkImageMemoryBarrier2 barrier{};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
  barrier.srcStageMask = srcStage;
  barrier.srcAccessMask = srcAccess;
  barrier.dstStageMask = dstStage;
  barrier.dstAccessMask = dstAccess;
  barrier.oldLayout = oldLayout;
  barrier.newLayout = newLayout;
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.image = image;
  barrier.subresourceRange = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1};

  VkDependencyInfo dependency{};
  dependency.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
  dependency.imageMemoryBarrierCount = 1;
  dependency.pImageMemoryBarriers = &barrier;
  vkCmdPipelineBarrier2(cmd, &dependency);
};

/*=============================================================================
  Surface
  =============================================================================*/
void MRenderer::selectSurfaceFormat() {
  pDebugger->print("Selecting surface format . . .");

  // Error handling
  if (surfaceFormats.empty())
    throw std::runtime_error(
        "Cannot choose swapchain format: surfaceFormats vector is empty.");

  for (const auto &availableFormat : surfaceFormats) {
    if ((availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB ||
         availableFormat.format == VK_FORMAT_R8G8B8A8_SRGB) &&
        availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
      pDebugger->print("Selected optimal sRGB surface format.");
      surfaceFormat = availableFormat;
      return;
        };
  };

  // Fallback
  pDebugger->print("Preferred sRGB format not found. Falling back to first available format.",
                   MESSAGE_TYPE::WARNING);
  surfaceFormat = surfaceFormats[0];
};

void MRenderer::selectPresentMode() {
  pDebugger->print("Selecting present mode . . .");

  // Error handling
  if (surfacePresentModes.empty())
    throw std::runtime_error(
        "Cannot choose swapchain present mode: presentModes vector is empty.");

  // Look for the optimal present mode
  for (const auto &availablePresentMode : surfacePresentModes) {
    if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
      surfacePresentMode = availablePresentMode;
      pDebugger->print("Selected present mode VK_PRESENT_MODE_MAILBOX_KHR.");
      return;
    };
  };

  // Fallback if the MAILBOX present mode is not available
  pDebugger->print("Couldn't find the wanted present mode fallback was chosen: "
                   "VK_PRESENT_MODE_FIFO_KHR.",
                   MESSAGE_TYPE::WARNING);
  surfacePresentMode =  VK_PRESENT_MODE_FIFO_KHR;
};

/*=============================================================================
  Command pool
  =============================================================================*/
void MRenderer::createCommandPool(const uint32_t graphicsQueueFamilyIndex) {
  pDebugger->print("Creating command pool, queue family index:, " +
                   std::to_string(graphicsQueueFamilyIndex) + " . . .");

  VkCommandPoolCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  createInfo.queueFamilyIndex = graphicsQueueFamilyIndex;
  createInfo.flags = 0;

  if (vkCreateCommandPool(logicalDevice, &createInfo, nullptr, &commandPool) !=
      VK_SUCCESS) {
    throw std::runtime_error("Failed to create command pool!");
  };

  pDebugger->print("Created command pool.");
};

void MRenderer::destroyCommandPool() const {
  pDebugger->print("Destroying command pool . . .");

  vkDestroyCommandPool(logicalDevice, commandPool, nullptr);

  pDebugger->print("Destroyed command pool.");
};

/*=============================================================================
  Command buffer
  =============================================================================*/
void MRenderer::createCommandBuffer() {
  pDebugger->print("Creating command buffer . . .");

  VkCommandBufferAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.commandPool = commandPool;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandBufferCount = 1;

  if (vkAllocateCommandBuffers(logicalDevice, &allocInfo, &commandBuffer) != VK_SUCCESS)
    throw std::runtime_error("Failed to allocate command buffer!");
};

void MRenderer::recordCommandBuffer() const {
  vkResetCommandPool(logicalDevice, commandPool, 0);

  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
  if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) throw std::runtime_error("Failed to begin recording command buffer!");

  transitionImage(commandBuffer, swapchainImages[imageIndex],
                  VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                  VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, 0,
                  VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                  VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT);


  VkRenderingAttachmentInfo colorAttachment{};
  colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
  colorAttachment.imageView = swapchainImageViews[imageIndex];
  colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  colorAttachment.clearValue.color = {{0.0f, 0.0f, 0.0f, 1.0f}};

  VkRenderingInfo renderingInfo{};
  renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
  renderingInfo.renderArea = {{0, 0}, extent};
  renderingInfo.layerCount = 1;
  renderingInfo.colorAttachmentCount = 1;
  renderingInfo.pColorAttachments = &colorAttachment;

  vkCmdBeginRendering(commandBuffer, &renderingInfo);

  vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

  const VkViewport viewport{.x = 0.0f, .y = 0.0f, .width = static_cast<float>(extent.width),.height = static_cast<float>(extent.height), .minDepth = 0.0f, .maxDepth = 1.0f};
  const VkRect2D scissor{.offset = {0, 0}, .extent = extent};
  vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
  vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

  vkCmdDraw(commandBuffer, 3, 1, 0, 0);

  vkCmdEndRendering(commandBuffer);

  transitionImage(commandBuffer, swapchainImages[imageIndex],
                  VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                  VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                  VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
                  VK_PIPELINE_STAGE_2_NONE, 0);

  if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
    throw std::runtime_error("Failed to end recording command buffer!");
}

/*=============================================================================
  Semaphores
  =============================================================================*/
void MRenderer::createSemaphores() {
  pDebugger->print("Creating semaphores . . .");

  VkSemaphoreCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

  //Error handling
  if (vkCreateSemaphore(logicalDevice, &createInfo, nullptr,&semaphoreImageAvailable) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create semaphore for checking image availability!") ;
  };
  if (vkCreateSemaphore(logicalDevice, &createInfo, nullptr,&semaphoreRendererFinished) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create semaphore for checking if the renderer is finished!") ;
  }
};

void MRenderer::destroySemaphores() const {
  pDebugger->print("Destroying semaphores . . .");

  vkDestroySemaphore(logicalDevice, semaphoreImageAvailable, nullptr);
  vkDestroySemaphore(logicalDevice, semaphoreRendererFinished, nullptr);
};

/*=============================================================================
  Graphics pipeline
  =============================================================================*/
void MRenderer::createGraphicsPipeline() {
  pDebugger->print("Creating graphics pipeline . . .");

  if (vertShaderModule == VK_NULL_HANDLE || fragShaderModule == VK_NULL_HANDLE) {
    pDebugger->print("Setting fragment & vertex shader info.");

    const auto vertShaderCode = readShaderFile("../include/Magma/Shaders/vert.spv");
    const auto fragShaderCode = readShaderFile("../include/Magma/Shaders/frag.spv");

    createShaderModule(vertShaderCode,vertShaderModule);
    createShaderModule(fragShaderCode,fragShaderModule);

    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";
    vertShaderStageInfo.pSpecializationInfo = nullptr;

    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";
    fragShaderStageInfo.pSpecializationInfo = nullptr;
  };
  VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};
  //Vertex input stage
  VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
  {
    pDebugger->print("Setting vertex input info.");
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 0;
    vertexInputInfo.pVertexBindingDescriptions = nullptr;
    vertexInputInfo.vertexAttributeDescriptionCount = 0;
    vertexInputInfo.pVertexAttributeDescriptions = nullptr;
  };

  //Input assembly
  VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
  {
    pDebugger->print("Creating input assembly.");
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;
  };

  //Viewport state
  VkPipelineViewportStateCreateInfo viewportState{};
  {
    pDebugger->print("Creating viewport state.");

    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;
  };

  // Dynamic rendering
  constexpr VkDynamicState dynamicStates[]{VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
  VkPipelineDynamicStateCreateInfo dynamicState{};
  {
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = 2;
    dynamicState.pDynamicStates = dynamicStates;
  };

  //Attachments
  const VkFormat colorFormat = surfaceFormat.value().format;
  VkPipelineRenderingCreateInfo renderingCreateInfo{};
  {
    renderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
    renderingCreateInfo.colorAttachmentCount = 1;
    renderingCreateInfo.pColorAttachmentFormats = &colorFormat;
  };

  //Rasterizer
  VkPipelineRasterizationStateCreateInfo rasterizer{};
  {
    pDebugger->print("Creating rasterizer.");

    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_NONE;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f;
    rasterizer.depthBiasClamp = 0.0f;
    rasterizer.depthBiasSlopeFactor = 0.0f;
  };

  //Multisampling
  VkPipelineMultisampleStateCreateInfo multisampling{};
  {
    pDebugger->print("Setting multisampling info.");

    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f;
    multisampling.pSampleMask = nullptr;
    multisampling.alphaToCoverageEnable = VK_FALSE;
    multisampling.alphaToOneEnable = VK_FALSE;
  };

  //Color blending
  VkPipelineColorBlendAttachmentState colorBlendAttachment{};
  VkPipelineColorBlendStateCreateInfo colorBlending{};
  {
    pDebugger->print("Setting color blending.");

    //Blend attachment
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
      VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT |
          VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
    //Blend state
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;
  };


  //Final pipeline creation
  VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
  {
    pDebugger->print("Creating pipeline layout info.");

    pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutCreateInfo.setLayoutCount = 0;
    pipelineLayoutCreateInfo.pSetLayouts = nullptr;
    pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
    pipelineLayoutCreateInfo.pPushConstantRanges = nullptr;

    //Error handling
    if (vkCreatePipelineLayout(logicalDevice, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout) != VK_SUCCESS){
      throw std::runtime_error("Failed to create pipeline layout!");
    };

    pDebugger->print("Setting pipeline creation info.");
    VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
    pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineCreateInfo.stageCount = 2;
    pipelineCreateInfo.pStages = shaderStages;
    pipelineCreateInfo.pVertexInputState = &vertexInputInfo;
    pipelineCreateInfo.pInputAssemblyState = &inputAssembly;
    pipelineCreateInfo.pViewportState = &viewportState;
    pipelineCreateInfo.pRasterizationState = &rasterizer;
    pipelineCreateInfo.pMultisampleState = &multisampling;
    pipelineCreateInfo.pDepthStencilState = nullptr;
    pipelineCreateInfo.pColorBlendState = &colorBlending;
    pipelineCreateInfo.pDynamicState = &dynamicState;
    pipelineCreateInfo.layout = pipelineLayout;
    pipelineCreateInfo.renderPass = VK_NULL_HANDLE;
    pipelineCreateInfo.subpass = 0;
    pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineCreateInfo.basePipelineIndex = -1;
    pipelineCreateInfo.pNext = &renderingCreateInfo;
    if (vkCreateGraphicsPipelines(logicalDevice,VK_NULL_HANDLE,1,&pipelineCreateInfo,nullptr,&graphicsPipeline) != VK_SUCCESS) throw std::runtime_error("Failed to create graphics pipeline!");

    pDebugger->print("Created graphics pipeline.");
  };
};

void MRenderer::destroyGraphicsPipeline() const {
  pDebugger->print("Destroying graphics pipeline.");

  vkDestroyPipeline(logicalDevice,graphicsPipeline,nullptr);
  vkDestroyPipelineLayout(logicalDevice, pipelineLayout, nullptr);
};

/*=============================================================================
  Shaders
  =============================================================================*/
std::vector<char> MRenderer::readShaderFile(const std::string &fileName) const {
  pDebugger->print("Attempting to read the shader file from: " + fileName + ".");

  std::ifstream file(fileName,std::ios::ate | std::ios::binary);

  if (!file.is_open()) throw std::runtime_error("Failed to open shader file! Current working directory: " + std::filesystem::current_path().string());

  const long fileSize = file.tellg();
  std::vector<char> buffer(fileSize);
  file.seekg(0);

  file.read(buffer.data(), fileSize);
  pDebugger->print("Reading shader file:" + fileName + ".");
  file.close();

  pDebugger->print("Finished reading the shader file from: " + fileName + ".");
  return buffer;
};

void MRenderer::createShaderModule(const std::vector<char> &code,VkShaderModule &shaderModule) const {
  pDebugger->print("Creating shader module . . .");

  VkShaderModuleCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  createInfo.codeSize = code.size();
  createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());

  //Error handling
  if (vkCreateShaderModule(logicalDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) throw std::runtime_error("Failed to create shader module!");

  pDebugger->print("Created shader module.");
};

void MRenderer::destroyShaderModule() const {
  vkDestroyShaderModule(logicalDevice,vertShaderModule,nullptr);
  vkDestroyShaderModule(logicalDevice,fragShaderModule,nullptr);

  pDebugger->print("Destroyed shader modules.");
};