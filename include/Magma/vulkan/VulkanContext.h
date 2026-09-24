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

        //Debugging
        MDebugger* pDebugger{nullptr};
    };
}
