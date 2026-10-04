#pragma once

#include "../Uitls/Debugger.h"

#include <vulkan/vulkan.h>

namespace Magma {
    class MSwapchain {
    public:
        explicit MSwapchain(MDebugger& debugger,VkPhysicalDevice physicalDevice,VkSurfaceKHR surface);
        ~MSwapchain();

        /*# =============================================================================
         #  Swapchain
         # ============================================================================= */
        void querySupport();
        [[nodiscard]] VkSurfaceFormatKHR chooseFormat() const;
        [[nodiscard]] VkPresentModeKHR  choosePresentMode() const;
         void create();
    private:
        /*# =============================================================================
          #  Debuging
          # ============================================================================= */
           MDebugger *pDebugger;

        /*# =============================================================================
          #  Swapchain
          # ============================================================================= */
        VkSurfaceCapabilitiesKHR surfaceCapabilities;
        std::vector<VkSurfaceFormatKHR> surfaceFormats;
        std::vector<VkPresentModeKHR> presentModes;

        VkSurfaceKHR surface;
        VkPhysicalDevice physicalDevice;
    };
};