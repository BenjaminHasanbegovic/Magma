#pragma once

#include "../Uitls/Debugger.h"
#include "../Uitls/ErrorCodes.h"
#include <vulkan/vulkan.h>

namespace Magma {
    class MSwapchain {
    public:
        /* # =============================================================================
           #  Class init and destruction
           # ============================================================================= */
        explicit MSwapchain(MDebugger& debugger, const VkPhysicalDevice& physicalDevice,const VkDevice& logicalDevice,const VkSurfaceKHR& surface);
        ~MSwapchain();

        /*# =============================================================================
         #  Swapchain
         # ============================================================================= */
        void querySupport();
        void create();
        void setExtent(uint32_t width, uint32_t height);

    private:
        /*# =============================================================================
          #  Debuging
          # ============================================================================= */
           MDebugger *pDebugger;

        /*# =============================================================================
          #  Swapchain
          # ============================================================================= */
        VkSurfaceCapabilitiesKHR vkSurfaceCapabilities{};
        std::vector<VkSurfaceFormatKHR> vkSurfaceFormats;
        std::vector<VkPresentModeKHR> vkPresentModes;

        [[nodiscard]] VkSurfaceFormatKHR selectVkFormat() const;
        [[nodiscard]] VkPresentModeKHR  selectVkPresentMode() const;

        VkSwapchainKHR vkSwapchain{VK_NULL_HANDLE};
        unsigned int vkSwapchainImageCount{2};
        std::vector<VkImage> vkSwapchainImages;
        /*# =============================================================================
          #  Vulkan info
          # ============================================================================= */
        VkSurfaceKHR pVkSurface{VK_NULL_HANDLE};
        VkPhysicalDevice pVkPhysicalDevice{VK_NULL_HANDLE};
        VkDevice pVkLogicalDevice{VK_NULL_HANDLE};
    };
};