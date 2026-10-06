#include "../../../include/Magma/vulkan/Swapchain.h"

using namespace Magma;

/*=============================================================================
  Class init and destruction
  =============================================================================*/
MSwapchain::MSwapchain(MDebugger &debugger,
                       const VkPhysicalDevice &physicalDevice,
                       const VkDevice &logicalDevice,
                       const VkSurfaceKHR &surface) {
  // Set needed values
  pPhysicalDevice = physicalDevice;
  pLogicalDevice = logicalDevice;
  pSurface = surface;
  pDebugger = &debugger;

  pDebugger->print("Initializing swapchain.");
};

MSwapchain::~MSwapchain() {
  pDebugger->print("Destroying swapchain.");

  vkDestroySwapchainKHR(pLogicalDevice, Swapchain, nullptr);
  pDebugger = nullptr;
  pLogicalDevice = VK_NULL_HANDLE;
  pSurface = VK_NULL_HANDLE;
  pPhysicalDevice = VK_NULL_HANDLE;
};

/*=============================================================================
  Swapchain
  =============================================================================*/
void MSwapchain::querySupport() {
  pDebugger->print("Creating swapchain . . .");

  auto getSurfaceCapabilities = [this]() {
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(pPhysicalDevice, pSurface,
                                              &surfaceCapabilities);
    pDebugger->print("Fetched surface capabilities.");
  };

  auto getSurfaceFormats = [this]() {
    uint32_t surfaceFormatCount{0};

    vkGetPhysicalDeviceSurfaceFormatsKHR(pPhysicalDevice, pSurface,
                                         &surfaceFormatCount, nullptr);
    if (surfaceFormatCount == 0) {
      throw std::runtime_error("No surface format found.");
    }

    surfaceFormats.resize(surfaceFormatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(pPhysicalDevice, pSurface,
                                         &surfaceFormatCount,
                                         surfaceFormats.data());

    pDebugger->print("Fetched surface formats.");
  };

  auto getSurfacePresentMode = [this]() {
    uint32_t presentModeCount = 0;
    VkResult result = vkGetPhysicalDeviceSurfacePresentModesKHR(
        pPhysicalDevice, pSurface, &presentModeCount, nullptr);

    if (result != VK_SUCCESS) {
      throw std::runtime_error("Failed to query surface present mode count.");
    };

    if (presentModeCount == 0) {
      throw std::runtime_error("No surface present modes available.");
    };

    presentModes.resize(presentModeCount);
    result = vkGetPhysicalDeviceSurfacePresentModesKHR(
        pPhysicalDevice, pSurface, &presentModeCount, presentModes.data());
    if (result == VK_SUCCESS) {
      pDebugger->print("Fetched surface present modes.");
    } else {
      throw std::runtime_error("Failed to retrieve surface present modes.");
    };
  };

  getSurfaceCapabilities();
  getSurfaceFormats();
  getSurfacePresentMode();

  pDebugger->print("Created swapchain.");
};

VkSurfaceFormatKHR MSwapchain::selectVkFormat() const {
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
      return availableFormat;
    };
  };

  // Fallback
  pDebugger->print("Preferred sRGB format not found. Falling back to first "
                   "available format.",
                   MESSAGE_TYPE::WARNING);
  return surfaceFormats[0];
}

VkPresentModeKHR MSwapchain::selectVkPresentMode() const {
  pDebugger->print("Selecting present mode . . .");

  // Error handling
  if (presentModes.empty())
    throw std::runtime_error(
        "Cannot choose swapchain present mode: presentModes vector is empty.");

  // Look for the optimal present mode
  for (const auto &availablePresentMode : presentModes) {
    if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
      return availablePresentMode;
      pDebugger->print("Selected present mode VK_PRESENT_MODE_MAILBOX_KHR.");
      break;
    };
  };

  // Fallback if the MAILBOX present mode is not available
  pDebugger->print("Couldn't find the wanted present mode fallback was chosen: "
                   "VK_PRESENT_MODE_FIFO_KHR.",
                   MESSAGE_TYPE::WARNING);
  return VK_PRESENT_MODE_FIFO_KHR;
};

void MSwapchain::setExtent(const uint32_t width, const uint32_t height) {
  pDebugger->print("Setting surface extent . . .");

  surfaceCapabilities.currentExtent.width = width;
  surfaceCapabilities.currentExtent.height = height;

  pDebugger->print("Set the surface extent.");
};

void MSwapchain::create() {
  pDebugger->print("Creating swapchain . . .");

  const auto [format, colorSpace] = selectVkFormat();

  swapchainImageCount = surfaceCapabilities.minImageCount + 1;
  if (surfaceCapabilities.maxImageCount > 0 &&
      swapchainImageCount > surfaceCapabilities.maxImageCount) {
    swapchainImageCount = surfaceCapabilities.maxImageCount;
  };

  VkSwapchainCreateInfoKHR createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  createInfo.surface = pSurface;
  createInfo.minImageCount = swapchainImageCount;
  createInfo.imageFormat = format;
  createInfo.imageColorSpace = colorSpace;
  createInfo.imageArrayLayers = 1;
  createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  createInfo.preTransform = surfaceCapabilities.currentTransform;
  createInfo.imageExtent = surfaceCapabilities.currentExtent;
  createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  createInfo.presentMode = selectVkPresentMode();
  createInfo.compositeAlpha =
      VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; // Reminder: Move to window handler
                                         // logic
  createInfo.clipped = VK_TRUE;
  createInfo.oldSwapchain = VK_NULL_HANDLE;

  // Creating the swapchain
  if (vkCreateSwapchainKHR(pLogicalDevice, &createInfo, nullptr,
                           &Swapchain) != VK_SUCCESS) {
    throw std::runtime_error(
        "Failed to create Vulkan Swapchain!" +
        errorCodeToString(ErrorCode::SWAPCHAIN_CREATION_FAILED));
  };

  // Retrieve the handle of swapchain image
  vkGetSwapchainImagesKHR(pLogicalDevice, Swapchain, &swapchainImageCount,
                          nullptr);
  swapchainImages.resize(swapchainImageCount);
  vkGetSwapchainImagesKHR(pLogicalDevice, Swapchain, &swapchainImageCount,
                          swapchainImages.data());

  pDebugger->print("Created swapchain.");
};
