#pragma once

#include "../Uitls/Debugger.h"
#include "../Uitls/ErrorCodes.h"
#include <string>
#include <vector>
#include <optional>
#include <vulkan/vulkan.h>

      /* # =============================================================================
         #  Os handling
         # ============================================================================= */
#ifdef __linux__
//Linux handle
#include <xcb/xcb.h>
#include <vulkan/vulkan_xcb.h>
inline std::vector<const char*> instanceExtensions = {
    VK_KHR_SURFACE_EXTENSION_NAME,
    VK_KHR_XCB_SURFACE_EXTENSION_NAME
};
#elif defined(_WIN32) || defined(_WIN64)
//Window handle
#endif

namespace Magma {
    /* # =============================================================================
          #  External classes definition
          # ============================================================================= */
    class MDebugger;

    /* # =============================================================================
        #  Main class
        # ============================================================================= */
    class MVulkanContext {
    public:
        /* # =============================================================================
           #  Class init and destruction
           # ============================================================================= */
        MVulkanContext(const std::string& appName,MDebugger& debugger);
        ~MVulkanContext();

    private:

        /* # =============================================================================
           #  Magma instance
           # ============================================================================= */
        const char *pAppName{nullptr};
        MDebugger* pDebugger{nullptr};

        /*# =============================================================================
          #  Vulkan instance
         # ============================================================================= */
        VkInstance vkInstance = VK_NULL_HANDLE;
        void createInstance();
        void destroyVulkanInstance();

        /*# =============================================================================
          #  Logical device
          # ============================================================================= */
        VkDevice vkLogicalDevice = VK_NULL_HANDLE;
        void createLogicalDevice();
        void destroyLogicalDevice();

        /*# =============================================================================
          #  Physical device
          # ============================================================================= */
        VkPhysicalDevice vkPhysicalDevice = VK_NULL_HANDLE;
        VkPhysicalDeviceProperties vkPhysicalDeviceProperties{};
        VkPhysicalDeviceFeatures vkPhysicalDeviceFeatures{};
        void selectPhysicalDevice();
        VkPhysicalDevice scorePhysicalDevices(const std::vector<VkPhysicalDevice> &scorablePhysicalDevicesList);

        /*# =============================================================================
          #  Device queue
          # ============================================================================= */
        static std::optional<uint32_t> graphicsQueueFamilyIndex;
        static std::optional<uint32_t> presentQueueFamilyIndex;
        void findMandatoryQueueFamilies(const VkPhysicalDevice &physicalDevice) const;

        /*# =============================================================================
          #  Swapchain
          # ============================================================================= */
            VkSurfaceCapabilitiesKHR capabilities{};
            std::vector<VkSurfaceFormatKHR> formats;
            std::vector<VkPresentModeKHR> presentModes;

            [[nodiscard]] bool isSuitable() const {
                return !formats.empty() && !presentModes.empty();
            }


        /*# =============================================================================
         #  Error handling
         # ============================================================================= */
        static ErrorCode errorCode;
        bool checkInstanceCreationErrors(VkResult result) const;
    };
};
