#pragma once

#include "../Debugger.h"
class MDebugger;

#include <vulkan/vulkan.h>

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
        static bool checkInstanceCreationErrors(VkResult result);
        void destroyVulkanInstance();

        //Device logic
        VkDevice vkLogicalDevice = VK_NULL_HANDLE;
        VkPhysicalDevice vkPhysicalDevice = VK_NULL_HANDLE;
        void selectPhysicalDevice();
        static VkPhysicalDevice scorePhysicalDevices(const std::vector<VkPhysicalDevice> &scorablePhysicalDevicesList);

        //Debugging
        MDebugger* pDebugger{nullptr};
    };
}
