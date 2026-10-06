#pragma once

#include "../Uitls/Debugger.h"
#include <vulkan/vulkan.h>

namespace Magma {
    class Renderer {
    public:
        /*# =============================================================================
          #  Swapchain
          # ============================================================================= */
        VkSwapchainKHR pVkSwapchain{VK_NULL_HANDLE};
        uint32_t pVkSwapchainImageCount{2};
        std::vector<VkImage> pVkSwapchainImages;

    private:
    };
};