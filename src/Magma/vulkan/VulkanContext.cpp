#include "../../../include/Magma/vulkan/VulkanContext.h"

using namespace Magma;

MVulkanContext::MVulkanContext(const std::string& appName,MDebugger& debugger){
    pAppName = appName.c_str();
    pDebugger = &debugger;

    createInstance();
};

MVulkanContext::~MVulkanContext() {
    MDebugger::print("Destroying vulkan context . . .");
    destroyVulkanInstance();
    MDebugger::print("Destroyed vulkan context.");
};

//Vulkan instance logic
void MVulkanContext::createInstance() {
    MDebugger::print("Creating vulkan instance . . .");

    MDebugger::print("Generating app info . . .");
    VkApplicationInfo appInfo {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.apiVersion = VK_MAKE_VERSION(1 , 0 , 0);
    appInfo.applicationVersion = VK_MAKE_VERSION(1 , 0 , 0);
    appInfo.pApplicationName = pAppName;
    appInfo.pNext = nullptr;
    MDebugger::print("Generated app info.");

    MDebugger::print("Generating instance create info . . .");
    VkInstanceCreateInfo instanceCreateInfo {};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pApplicationInfo = &appInfo;
    instanceCreateInfo.enabledLayerCount = 0;
    instanceCreateInfo.ppEnabledLayerNames = nullptr;
    instanceCreateInfo.enabledExtensionCount = 0;
    instanceCreateInfo.ppEnabledExtensionNames = nullptr;
    instanceCreateInfo.pNext = nullptr;
    MDebugger::print("Generated instance create info.");

    if (checkInstanceCreationErrors(vkCreateInstance(&instanceCreateInfo, nullptr, &vkInstance))) {
        MDebugger::print("Created vulkan instance.");
    }else {
        MDebugger::print("Failed to create vulkan instance.");

    }
}

bool MVulkanContext::checkInstanceCreationErrors(const VkResult result) {
    MDebugger::print("Checking instance creation for errors . . .");
    if (result == VK_SUCCESS) {
        MDebugger::print("No errors found.");
        return true;
    } else {
        switch (result) {
            case VK_ERROR_INCOMPATIBLE_DRIVER:
                MDebugger::print("The Vulkan loader cannot find a driver that supports the requested API version or hardware requirements.",MESSAGE_TYPE::ERROR);
                break;

            case VK_ERROR_OUT_OF_HOST_MEMORY:
                MDebugger::print("The host CPU system ran out of system memory.",MESSAGE_TYPE::ERROR);
            break;

            case VK_ERROR_TOO_MANY_OBJECTS:
                MDebugger::print("Exceeded maximum number of active Vulkan instances supported by the driver.",MESSAGE_TYPE::ERROR);
                break;

            case VK_ERROR_OUT_OF_DEVICE_MEMORY:
                MDebugger::print("The device ran out of memory allocated for system-level operations.",MESSAGE_TYPE::ERROR);
                break;

                case VK_ERROR_INITIALIZATION_FAILED:
                MDebugger::print("The Vulkan loader or implementation failed to initialize for an unspecified internal reason.",MESSAGE_TYPE::ERROR);
                break;

            case VK_ERROR_LAYER_NOT_PRESENT:
                MDebugger::print("One or more requested instance validation layers could not be found.",MESSAGE_TYPE::ERROR);
                break;

            case VK_ERROR_EXTENSION_NOT_PRESENT:
                MDebugger::print("One or more requested instance extensions are not supported by the loader or installed drivers.",MESSAGE_TYPE::ERROR);
                break;

            default:
                MDebugger::print("An unknown critical error occurred within the Vulkan driver or loader.",MESSAGE_TYPE::ERROR);
                break;
        }
        return false;
    }
}

void MVulkanContext::destroyVulkanInstance() {
    vkDestroyInstance(vkInstance, nullptr);
    vkInstance = VK_NULL_HANDLE;
    MDebugger::print("Destroyed vulkan instance.");
}
