#pragma once

#include "../Uitls/Debugger.h"

#include <vulkan/vulkan.h>

namespace Magma {
    class MSwapchain {
    public:
        explicit MSwapchain(MDebugger& debugger);
        ~MSwapchain();

        /*# =============================================================================
         #  Swapchain
         # ============================================================================= */
        void create(VkPhysicalDevice physicalDevice,VkSurfaceKHR surface);
        VkSurfaceFormatKHR chooseSwapchainFormat() const;
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
    };
};