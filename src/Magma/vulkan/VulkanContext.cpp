#include "../../../include/Magma/vulkan/VulkanContext.h"

using namespace Magma;

MVulkanContext::MVulkanContext(const std::string& appName,MDebugger& debugger){
    pAppName = appName.c_str();
    pDebugger = &debugger;

    createInstance();
    selectPhysicalDevice();
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

//Device logic

void MVulkanContext::selectPhysicalDevice() {
    MDebugger::print("Selecting a physical device.");
    //Getting the data of all physical devices
    MDebugger::print("Getting all physical device . . .");

    uint32_t physicalDeviceCount = 0;
    vkEnumeratePhysicalDevices(vkInstance, &physicalDeviceCount, nullptr);

    if (physicalDeviceCount > 0) {
        MDebugger::print("Found " + std::to_string(physicalDeviceCount) + " physical device(s).");
    }else {
        MDebugger::print("Failed to find physical device with Vulkan support!",MESSAGE_TYPE::ERROR);
    }

    std::vector<VkPhysicalDevice> physicalDevicesList(physicalDeviceCount);
    vkEnumeratePhysicalDevices(vkInstance, &physicalDeviceCount, physicalDevicesList.data());

    //Selecting a suitable physical device
    vkPhysicalDevice = scorePhysicalDevices(physicalDevicesList);
}

VkPhysicalDevice MVulkanContext::scorePhysicalDevices(const std::vector<VkPhysicalDevice> &scorablePhysicalDevicesList){
    MDebugger::print("Scoring a physical devices . . .");

    unsigned int maxPhysicalDeviceScore{0};
    VkPhysicalDevice suitablePhysicalDevice{VK_NULL_HANDLE};
    for (const auto physicalDevice : scorablePhysicalDevicesList) {
        unsigned int currentPhysicalDeviceScore{0};
    MDebugger::print("Scoring physical device . . .");

    //Physical device properties.
    MDebugger::print("Getting the physical device properties . . .");

    VkPhysicalDeviceProperties physicalDeviceProperties{};
    vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);
        const std::string deviceName = physicalDeviceProperties.deviceName;
    const VkPhysicalDeviceType physicalDeviceType = physicalDeviceProperties.deviceType;

    MDebugger::print("Got the physical device properties, name:" + deviceName);

    //Physical device features.
    MDebugger::print("Getting the physical device features . . .");

    VkPhysicalDeviceFeatures physicalDeviceFeatures{};
    vkGetPhysicalDeviceFeatures(physicalDevice, &physicalDeviceFeatures);

    MDebugger::print("Got the physical device features.");

        //Scoring logic
        //Mandatory properties and features
        if (!physicalDeviceFeatures.samplerAnisotropy) {
            MDebugger::print("The physical device isn't suitable,doesn't support sampler anisotropy.", MESSAGE_TYPE::ERROR);
            currentPhysicalDeviceScore = 0;
            continue;
        }

        if (!physicalDeviceFeatures.independentBlend) {
            MDebugger::print("The physical device isn't suitable,doesn't support independent blend.", MESSAGE_TYPE::ERROR);
            currentPhysicalDeviceScore = 0;
            continue;
        }

        if (!physicalDeviceFeatures.fillModeNonSolid) {
            MDebugger::print("The physical device isn't suitable,doesn't support fillModeNonSolid.", MESSAGE_TYPE::ERROR);
            currentPhysicalDeviceScore = 0;
            continue;
        }

        if (!physicalDeviceFeatures.depthClamp) {
            MDebugger::print("The physical device isn't suitable,doesn't support depth clamp", MESSAGE_TYPE::ERROR);
            currentPhysicalDeviceScore = 0;
            continue;
        }

        if (!physicalDeviceFeatures.geometryShader) {
            MDebugger::print("The physical device isn't suitable,doesn't support geometry shader.", MESSAGE_TYPE::ERROR);
            currentPhysicalDeviceScore = 0;
            continue;
        }

        //Optional properties  and features
        if (physicalDeviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            currentPhysicalDeviceScore += 1000;
            MDebugger::print("The physical device is a GPU, score:" + std::to_string(maxPhysicalDeviceScore));
        }

        currentPhysicalDeviceScore += physicalDeviceProperties.limits.maxImageDimension2D / 1000 + physicalDeviceProperties.limits.maxSamplerAnisotropy * 10 + physicalDeviceProperties.limits.maxBoundDescriptorSets; //Max 2D texture size + max anisotropic filtering level + how many descriptor sets can be bound at once
        MDebugger::print("The physical devices: max 2D texture size is " +std::to_string(physicalDeviceProperties.limits.maxImageDimension2D) + ", max anisotropic filtering level is:" + std::to_string(physicalDeviceProperties.limits.maxSamplerAnisotropy) + " and the max bound descriptor sets:" + std::to_string(physicalDeviceProperties.limits.maxBoundDescriptorSets) + ", score:" + std::to_string(maxPhysicalDeviceScore));


        //Final scoring
        if (currentPhysicalDeviceScore > maxPhysicalDeviceScore) {
        MDebugger::print("Found a new physical device properties, score: " + std::to_string(maxPhysicalDeviceScore) + ",name: " + deviceName);
      maxPhysicalDeviceScore = currentPhysicalDeviceScore;
            suitablePhysicalDevice = physicalDevice;
    }else if (currentPhysicalDeviceScore < maxPhysicalDeviceScore){
        MDebugger::print("Physical device had a worst score: " + std::to_string(maxPhysicalDeviceScore) + ",name: " + deviceName,MESSAGE_TYPE::WARNING);
    };
};
    MDebugger::print("Scored all physical devices.");
    if (suitablePhysicalDevice == VK_NULL_HANDLE) {
        MDebugger::print("Couldn't find a suitable physical device.",MESSAGE_TYPE::ERROR);
    }else {
        MDebugger::print("Found a suitable physical device.");
        return suitablePhysicalDevice;
    }
};
