#include "../../../include/Magma/vulkan/Swapchain.h"

using namespace Magma;

MSwapchain::MSwapchain(MDebugger& debugger) {
    pDebugger = &debugger;
    pDebugger->print("Initializing Swapchain.");
};

MSwapchain::~MSwapchain() {
    pDebugger->print("Destroying swapchain.");

    pDebugger = nullptr;
};

/*# =============================================================================
  #  Swapchain
  # ============================================================================= */

void MSwapchain::querySupport(VkPhysicalDevice physicalDevice,VkSurfaceKHR surface) {
    pDebugger->print("Creating swapchain . . .");

    auto getSurfaceCapabilities = [this](VkPhysicalDevice physicalDevice,VkSurfaceKHR surface) {
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCapabilities);
        pDebugger->print("Fetched surface capabilities.");
    };

    auto getSurfaceFormats = [this](VkPhysicalDevice physicalDevice,VkSurfaceKHR surface) {
        uint32_t surfaceFormatCount{0};

        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceFormatCount, nullptr);
        if (surfaceFormatCount == 0) {
            throw std::runtime_error("No surface format found.");
        }

        surfaceFormats.resize(surfaceFormatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceFormatCount, surfaceFormats.data());

        pDebugger->print("Fetched surface formats.");
    };

    auto getSurfacePresentMode = [this](VkPhysicalDevice physicalDevice, VkSurfaceKHR surface){
        uint32_t presentModeCount = 0;
        VkResult result = vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);

        if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to query surface present mode count.");
        };

        if (presentModeCount == 0) {
            throw std::runtime_error("No surface present modes available.");
        };

        presentModes.resize(presentModeCount);
        result = vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, presentModes.data());
        if (result == VK_SUCCESS) {
            pDebugger->print("Fetched surface present modes.");
        }else {
            throw std::runtime_error("Failed to retrieve surface present modes.");
        };
    };

    getSurfaceCapabilities(physicalDevice,surface);
    getSurfaceFormats(physicalDevice,surface);
    getSurfacePresentMode(physicalDevice,surface);

    pDebugger->print("Created swapchain.");
};

VkSurfaceFormatKHR MSwapchain::chooseFormat() const {
    pDebugger->print("Selecting surface format . . .");

    //Error handling
    if (surfaceFormats.empty()) throw std::runtime_error("Cannot choose swapchain format: surfaceFormats vector is empty.");

    for (const auto& availableFormat : surfaceFormats) {
        if ((availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB || availableFormat.format == VK_FORMAT_R8G8B8A8_SRGB) &&
             availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            pDebugger->print("Selected optimal sRGB surface format.");
            return availableFormat;
             };
    };

    //Fallback
    pDebugger->print("Preferred sRGB format not found. Falling back to first available format.", MESSAGE_TYPE::WARNING);
    return surfaceFormats[0];
}


VkPresentModeKHR MSwapchain::choosePresentMode() const {
    pDebugger->print("Selecting present mode . . .");

    //Error handling
    if (presentModes.empty()) throw std::runtime_error("Cannot choose swapchain present mode: presentModes vector is empty.");

    //Look for the optimal present mode
    for (const auto& availablePresentMode : presentModes) {
      if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
          return availablePresentMode;
          pDebugger->print("Selected present mode VK_PRESENT_MODE_MAILBOX_KHR.");
          break;
      };
  } ;

    //Fallback if the MAILBOX present mode is not available
    pDebugger->print("Couldn't find the wanted present mode fallback was chosen:VK_PRESENT_MODE_FIFO_KHR.", MESSAGE_TYPE::WARNING);
    return VK_PRESENT_MODE_FIFO_KHR;
};