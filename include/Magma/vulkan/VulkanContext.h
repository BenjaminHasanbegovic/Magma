#pragma once

#include "../Uitls/Debugger.h"
#include "../Uitls/ErrorCodes.h"
#include "../window/windowHandler.h"
#include <string>
#include <vector>
#include <optional>
#include <vulkan/vulkan.h>


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

        /*# =============================================================================
          #  Vulkan instance
          # ============================================================================= */
        void createInstance();

        /*# =============================================================================
          #  Logical device
          # ============================================================================= */
        void createLogicalDevice();

        /*# =============================================================================
          #  Physical device
          # ============================================================================= */
        void selectPhysicalDevice();

        /* # =============================================================================
           #  Os handling
           # ============================================================================= */
            static void setOSExtensions(USER_OS OS);


    private:

        /* # =============================================================================
           #  Magma instance
           # ============================================================================= */
        const char *pAppName{nullptr};
        MDebugger* pDebugger{nullptr};

      /* # =============================================================================
         #  Os handling
         # ============================================================================= */
      static std::vector<const char*> enabledExtensions;

        /*# =============================================================================
          #  Vulkan instance
         # ============================================================================= */
        VkInstance vkInstance = VK_NULL_HANDLE;
        void destroyVulkanInstance();

        /*# =============================================================================
          #  Logical device
          # ============================================================================= */
        VkDevice vkLogicalDevice = VK_NULL_HANDLE;
        void destroyLogicalDevice();

        /*# =============================================================================
          #  Physical device
          # ============================================================================= */
        VkPhysicalDevice vkPhysicalDevice = VK_NULL_HANDLE;
        VkPhysicalDeviceProperties vkPhysicalDeviceProperties{};
        VkPhysicalDeviceFeatures vkPhysicalDeviceFeatures{};
        VkPhysicalDevice scorePhysicalDevices(const std::vector<VkPhysicalDevice> &scorablePhysicalDevicesList) const;

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
        [[nodiscard]] bool checkInstanceCreationErrors(VkResult result) const;
    };
};
