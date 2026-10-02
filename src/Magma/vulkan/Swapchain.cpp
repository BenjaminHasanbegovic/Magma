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

void MSwapchain::create(VkPhysicalDevice physicalDevice,VkSurfaceKHR surface) {
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

VkSurfaceFormatKHR MSwapchain::chooseSwapchainFormat() const {
    pDebugger->print("Selecting optimal surface format . . .");

    if (surfaceFormats.empty()) {
        throw std::runtime_error("Cannot choose swapchain format: surfaceFormats vector is empty.");
    };

    for (const auto& availableFormat : surfaceFormats) {
        if ((availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB || availableFormat.format == VK_FORMAT_R8G8B8A8_SRGB) &&
             availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            pDebugger->print("Selected optimal sRGB surface format.");
            return availableFormat;
             };
    };

    pDebugger->print("Preferred sRGB format not found. Falling back to first available format.", MESSAGE_TYPE::WARNING);
    return surfaceFormats[0];
}
