#include "../../../include/Magma/vulkan/Swapchain.h"

#include <bits/fs_fwd.h>

using namespace Magma;

/* # =============================================================================
     #  Class init and destruction
     # ============================================================================= */
MSwapchain::MSwapchain(MDebugger& debugger, const VkPhysicalDevice& physicalDevice,const VkDevice& logicalDevice ,const VkSurfaceKHR& surface)
{
    //Set needed values
    pVkPhysicalDevice = physicalDevice;
    pVkLogicalDevice = logicalDevice;
    pVkSurface = surface;
    pDebugger = &debugger;

    pDebugger->print("Initializing swapchain.");
};

MSwapchain::~MSwapchain() {
    pDebugger->print("Destroying swapchain.");

    vkDestroySwapchainKHR(pVkLogicalDevice, vkSwapchain, nullptr);
    pDebugger = nullptr;
    pVkLogicalDevice = VK_NULL_HANDLE;
    pVkSurface = VK_NULL_HANDLE;
    pVkPhysicalDevice = VK_NULL_HANDLE;
};

/*# =============================================================================
  #  Swapchain
  # ============================================================================= */

void MSwapchain::querySupport() {
    pDebugger->print("Creating swapchain . . .");

    auto getSurfaceCapabilities = [this]() {
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(pVkPhysicalDevice, pVkSurface, &vkSurfaceCapabilities);
        pDebugger->print("Fetched surface capabilities.");
    };

    auto getSurfaceFormats = [this]() {
        uint32_t surfaceFormatCount{0};

        vkGetPhysicalDeviceSurfaceFormatsKHR(pVkPhysicalDevice, pVkSurface, &surfaceFormatCount, nullptr);
        if (surfaceFormatCount == 0) {
            throw std::runtime_error("No surface format found.");
        }

        vkSurfaceFormats.resize(surfaceFormatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(pVkPhysicalDevice, pVkSurface, &surfaceFormatCount, vkSurfaceFormats.data());

        pDebugger->print("Fetched surface formats.");
    };

    auto getSurfacePresentMode = [this](){
        uint32_t presentModeCount = 0;
        VkResult result = vkGetPhysicalDeviceSurfacePresentModesKHR(pVkPhysicalDevice, pVkSurface, &presentModeCount, nullptr);

        if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to query surface present mode count.");
        };

        if (presentModeCount == 0) {
            throw std::runtime_error("No surface present modes available.");
        };

        vkPresentModes.resize(presentModeCount);
        result = vkGetPhysicalDeviceSurfacePresentModesKHR(pVkPhysicalDevice, pVkSurface, &presentModeCount, vkPresentModes.data());
        if (result == VK_SUCCESS) {
            pDebugger->print("Fetched surface present modes.");
        }else {
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

    //Error handling
    if (vkSurfaceFormats.empty()) throw std::runtime_error("Cannot choose swapchain format: surfaceFormats vector is empty.");

    for (const auto& availableFormat : vkSurfaceFormats) {
        if ((availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB || availableFormat.format == VK_FORMAT_R8G8B8A8_SRGB) &&
             availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            pDebugger->print("Selected optimal sRGB surface format.");
            return availableFormat;
             };
    };

    //Fallback
    pDebugger->print("Preferred sRGB format not found. Falling back to first available format.", MESSAGE_TYPE::WARNING);
    return vkSurfaceFormats[0];
}

VkPresentModeKHR MSwapchain::selectVkPresentMode() const {
    pDebugger->print("Selecting present mode . . .");

    //Error handling
    if (vkPresentModes.empty()) throw std::runtime_error("Cannot choose swapchain present mode: presentModes vector is empty.");

    //Look for the optimal present mode
    for (const auto& availablePresentMode : vkPresentModes) {
      if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
          return availablePresentMode;
          pDebugger->print("Selected present mode VK_PRESENT_MODE_MAILBOX_KHR.");
          break;
      };
  } ;

    //Fallback if the MAILBOX present mode is not available
    pDebugger->print("Couldn't find the wanted present mode fallback was chosen: VK_PRESENT_MODE_FIFO_KHR.", MESSAGE_TYPE::WARNING);
    return VK_PRESENT_MODE_FIFO_KHR;
};

void MSwapchain::setExtent(const uint32_t width, const uint32_t height)  {
    pDebugger->print("Setting surface extent . . .");

     vkSurfaceCapabilities.currentExtent.width = width;
     vkSurfaceCapabilities.currentExtent.height = height;

    pDebugger->print("Set the surface extent.");
};

void MSwapchain::create() {
    pDebugger->print("Creating swapchain . . .");

    const auto [format, colorSpace] = selectVkFormat();

    vkSwapchainImageCount = vkSurfaceCapabilities.minImageCount + 1;
    if (vkSurfaceCapabilities.maxImageCount > 0 && vkSwapchainImageCount > vkSurfaceCapabilities.maxImageCount) {
        vkSwapchainImageCount = vkSurfaceCapabilities.maxImageCount;
    };

    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = pVkSurface;
    createInfo.minImageCount = vkSwapchainImageCount;
    createInfo.imageFormat = format;
    createInfo.imageColorSpace = colorSpace;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    createInfo.preTransform = vkSurfaceCapabilities.currentTransform;
    createInfo.imageExtent = vkSurfaceCapabilities.currentExtent;
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.presentMode =  selectVkPresentMode();
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; //Reminder: Move to window handler logic
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    // Creating the swapchain
    if (vkCreateSwapchainKHR(pVkLogicalDevice, &createInfo, nullptr, &vkSwapchain) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create Vulkan Swapchain!" + errorCodeToString(ErrorCode::SWAPCHAIN_CREATION_FAILED));
    };

//Retrieve the handle of swapchain image
    vkGetSwapchainImagesKHR(pVkLogicalDevice,vkSwapchain,&vkSwapchainImageCount,nullptr);
    vkSwapchainImages.resize(vkSwapchainImageCount);
    vkGetSwapchainImagesKHR(pVkLogicalDevice,vkSwapchain,&vkSwapchainImageCount,vkSwapchainImages.data());

    pDebugger->print("Created swapchain.");
};
