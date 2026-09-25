#pragma once

#include "../Debugger.h"
class MDebugger;

#include "../ErrorCodes.h"


#include <string>
#include <vector>
#include <optional>
#include <set>
#include <vulkan/vulkan.h>

#ifdef __linux__
#include <xcb/xcb.h>
#include <vulkan/vulkan_xcb.h>
inline std::vector<const char*> instanceExtensions = {
    VK_KHR_SURFACE_EXTENSION_NAME,
    VK_KHR_XCB_SURFACE_EXTENSION_NAME
};
#elif defined(_WIN32) || defined(_WIN64)
#include <vulkan/vulkan.h>
// ...
#endif

namespace Magma {
    class MVulkanContext {
    public:
        MVulkanContext(const std::string& appName,MDebugger& debugger);
        ~MVulkanContext();
    private:
        //Magma app info
        const char *pAppName{nullptr};

        //Vulkan instance logic
        VkInstance vkInstance = VK_NULL_HANDLE;
        void createInstance();
        bool checkInstanceCreationErrors(VkResult result);
        void destroyVulkanInstance();

        //Device logic
        VkDevice vkLogicalDevice = VK_NULL_HANDLE;
        void createLogicalDevice();
        void destroyLogicalDevice();
        VkPhysicalDevice vkPhysicalDevice = VK_NULL_HANDLE;
        VkPhysicalDeviceProperties vkPhysicalDeviceProperties{};
        VkPhysicalDeviceFeatures vkPhysicalDeviceFeatures{};
        void selectPhysicalDevice();
        VkPhysicalDevice scorePhysicalDevices(const std::vector<VkPhysicalDevice> &scorablePhysicalDevicesList);

        //Queue logic
        static std::optional<uint32_t> graphicsQueueFamilyIndex;
        static std::optional<uint32_t> presentQueueFamilyIndex;
        void findQueueFamilies(const VkPhysicalDevice &physicalDevice);

        //Swapchain logic
        struct SwapchainSupportDetails {
            VkSurfaceCapabilitiesKHR capabilities;
            std::vector<VkSurfaceFormatKHR> formats;
            std::vector<VkPresentModeKHR> presentModes;

            [[nodiscard]] bool isSuitable() const {
                return !formats.empty() && !presentModes.empty();
            }
        };

        //Error handling
        static ErrorCode errorCode;

        //Debugging
        MDebugger* pDebugger{nullptr};
    };
}
