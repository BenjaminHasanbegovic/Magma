#include "../../../include/Magma/Vulkan/Renderer.h"

using namespace Magma;

/*=============================================================================
  Class init and destruction
  =============================================================================*/
MRenderer::MRenderer(MDebugger &debugger,
                     const VkPhysicalDevice &physicalDevice,
                     const VkDevice &logicalDevice, const VkSurfaceKHR &surface)
    : Surface(surface), physicalDevice(physicalDevice),
      logicalDevice(logicalDevice), pDebugger(&debugger) {
  pDebugger->print("Constructed renderer .");
};

MRenderer::~MRenderer() {
  pDebugger->print("Destroying renderer . . .");

  destroyGraphicsPipeline();
  destroyCommandPool();
  destroyFramebuffers();
  destroyRenderPass();
  destroyImageViews();
  destroySwapchain();

  Swapchain = nullptr;
  pDebugger = nullptr;
};

/*=============================================================================
  Swapchain
  =============================================================================*/
void MRenderer::createSwapchain() {
pDebugger->print("Creating swapchain . . .");

  selectSurfaceFormat();
selectPresentMode();

  swapchainImageCount = surfaceCapabilities.minImageCount + 1;
  if (surfaceCapabilities.maxImageCount > 0 &&
      swapchainImageCount > surfaceCapabilities.maxImageCount) {
    swapchainImageCount = surfaceCapabilities.maxImageCount;
  };

  VkSwapchainCreateInfoKHR createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  createInfo.surface = Surface;
  createInfo.minImageCount = swapchainImageCount;
  createInfo.imageFormat = surfaceFormat.format;
  createInfo.imageColorSpace =  surfaceFormat.colorSpace;
  createInfo.imageArrayLayers = 1;
  createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  createInfo.preTransform = surfaceCapabilities.currentTransform;
  createInfo.imageExtent = surfaceCapabilities.currentExtent;
  createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  createInfo.presentMode = surfacePresentMode;
  createInfo.compositeAlpha =
      VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; // Reminder: Move to window handler
  // logic
  createInfo.clipped = VK_TRUE;
  createInfo.oldSwapchain = VK_NULL_HANDLE;

  // Creating the swapchain
  if (vkCreateSwapchainKHR(logicalDevice, &createInfo, nullptr, &Swapchain) !=
      VK_SUCCESS) {
    throw std::runtime_error("Failed to create Vulkan Swapchain!");
  };

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

/*=============================================================================
  Framebuffer
  =============================================================================*/
void MRenderer::createFramebuffers() {
  pDebugger->print("Creating framebuffers . . .");

  swapchainFramebuffers.resize(swapchainImageViews.size());

  for (size_t i = 0; i < swapchainImageViews.size(); i++) {
    const VkImageView attachments[] = {
        swapchainImageViews[i],
    };

    VkFramebufferCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    createInfo.renderPass = renderPass;
    createInfo.attachmentCount = 1;
    createInfo.pAttachments = attachments;
    createInfo.width = extent.width;
    createInfo.height = extent.height;
    createInfo.layers = 1;

    if (vkCreateFramebuffer(logicalDevice, &createInfo, nullptr,
                            &swapchainFramebuffers[i]) != VK_SUCCESS) {
      throw std::runtime_error("Failed to create framebuffers!");};
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

void MRenderer::createRenderPass() {
  pDebugger->print("Creating render pass . . .");

  // Color attachments
  pDebugger->print("Setting the color attachment to the render pass . . .");
  VkAttachmentDescription colorAttachment{};
  colorAttachment.format = surfaceFormat.format;
  colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
  colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  VkAttachmentReference colorAttachmentRef{};
  colorAttachmentRef.attachment = 0;
  colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  pDebugger->print("Set the color attachment.");

  // Subpass
  pDebugger->print("Setting the subpass to the render pass . . .");
  VkSubpassDescription subpass{};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &colorAttachmentRef;
  pDebugger->print("Set the sub pass.");

  // Final: render pass creation
  pDebugger->print("Setting the info for the render pass . . .");
  VkRenderPassCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  createInfo.attachmentCount = 1;
  createInfo.pAttachments = &colorAttachment;
  createInfo.subpassCount = 1;
  createInfo.pSubpasses = &subpass;
  createInfo.dependencyCount = 0;
  createInfo.pDependencies = nullptr;
  pDebugger->print("Set the info for the render pass.");

  if (vkCreateRenderPass(logicalDevice, &createInfo, nullptr, &renderPass) !=
      VK_SUCCESS) {
    throw std::runtime_error("Failed to create render pass!");
  };

  pDebugger->print("Created render pass.");
};

void MRenderer::destroyRenderPass() const {
  pDebugger->print("Destroying render pass . . .");
  vkDestroyRenderPass(logicalDevice, renderPass, nullptr);
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
      break;
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

void MRenderer::createCommandBuffers() {
  pDebugger->print("Creating command buffers . . .");

  commandBuffers.resize(swapchainImageViews.size());

  VkCommandBufferAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.commandPool = commandPool;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

  if (vkAllocateCommandBuffers(logicalDevice, &allocInfo,
                               commandBuffers.data()) != VK_SUCCESS) {
    throw std::runtime_error("Failed to allocate command buffers!");
  };

  pDebugger->print("Created command buffers .");
};

/*=============================================================================
  Graphics pipeline
  =============================================================================*/
void MRenderer::createGraphicsPipeline() {
  pDebugger->print("Creating graphics pipeline . . .");

  VkShaderModule vertShaderModule{VK_NULL_HANDLE};
  VkShaderModule fragShaderModule{VK_NULL_HANDLE};
  VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
  VkPipelineShaderStageCreateInfo fragShaderStageInfo{};

  {
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

  //Viewports and scissors
  VkViewport viewport{};
  VkRect2D scissor{};
  VkPipelineViewportStateCreateInfo viewportState{};
  {
    pDebugger->print("Creating viewport.");
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(extent.width);
    viewport.height = static_cast<float>(extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    scissor.offset = { .x = 0, .y = 0 };
    scissor.extent = extent;

    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;
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
    if (vkCreatePipelineLayout(logicalDevice, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout) != VK_SUCCESS) throw std::runtime_error("Failed to create pipeline layout!");
    };

  pDebugger->print("Setting pipeline createion info.");
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
  pipelineCreateInfo.pDynamicState = nullptr;
  pipelineCreateInfo.layout = pipelineLayout;
  pipelineCreateInfo.renderPass = renderPass;
  pipelineCreateInfo.subpass = 0;
  pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
  pipelineCreateInfo.basePipelineIndex = -1;

  if (vkCreateGraphicsPipelines(logicalDevice,VK_NULL_HANDLE,1,&pipelineCreateInfo,nullptr,&graphicsPipeline) != VK_SUCCESS) throw std::runtime_error("Failed to create graphics pipeline!");

  pDebugger->print("Created graphics pipeline.");
  //Cleanup
  vkDestroyShaderModule(logicalDevice,vertShaderModule,nullptr);
  vkDestroyShaderModule(logicalDevice,fragShaderModule,nullptr);

  pDebugger->print("Destroyed shader modules.");
};

void MRenderer::destroyGraphicsPipeline() const {
  pDebugger->print("Destroying graphics pipeline.");

  vkDestroyPipeline(logicalDevice,graphicsPipeline,nullptr);
};

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