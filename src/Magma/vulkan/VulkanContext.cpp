#include "../../../include/Magma/vulkan/VulkanContext.h"
#include "../../../include/Magma/Magma.h"

using namespace Magma;
//Explicit static values declaration
std::optional<uint32_t> MVulkanContext::graphicsQueueFamilyIndex = std::nullopt;
std::optional<uint32_t> MVulkanContext::presentQueueFamilyIndex = std::nullopt;
ErrorCode MVulkanContext::errorCode = ErrorCode::NO_ERRORS;

MVulkanContext::MVulkanContext(const std::string& appName,MDebugger& debugger){
    pAppName = appName.c_str();
    pDebugger = &debugger;

    createInstance();
    selectPhysicalDevice();
    createLogicalDevice();
};

MVulkanContext::~MVulkanContext() {
    pDebugger->print("Destroying vulkan context . . .");
    destroyLogicalDevice();
    destroyVulkanInstance();
    pDebugger->print("Destroyed vulkan context.");
};

//Vulkan instance logic
void MVulkanContext::createInstance() {
    pDebugger->print("Creating vulkan instance . . .");
    if (errorCode != ErrorCode::NO_ERRORS) {
        pDebugger->print("Cannot create vulkan instance.", MESSAGE_TYPE::ERROR);
        return;
    }

    pDebugger->print("Generating app info . . .");
    VkApplicationInfo appInfo {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.apiVersion = VK_API_VERSION_1_0;
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pApplicationName = pAppName;
    appInfo.pNext = nullptr;

    std::vector<const char*> enabledExtensions = {
        VK_KHR_SURFACE_EXTENSION_NAME
    };

#ifdef __linux__
    enabledExtensions.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
#elif defined(_WIN32) || defined(_WIN64)
    enabledExtensions.push_back("VK_KHR_win32_surface");
#endif

   pDebugger->print("Generating instance create info . . .");
    VkInstanceCreateInfo instanceCreateInfo {};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pApplicationInfo = &appInfo;
    instanceCreateInfo.enabledLayerCount = 0;
    instanceCreateInfo.ppEnabledLayerNames = nullptr;
    instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(enabledExtensions.size());
    instanceCreateInfo.ppEnabledExtensionNames = enabledExtensions.data();
    instanceCreateInfo.pNext = nullptr;

    if (checkInstanceCreationErrors(vkCreateInstance(&instanceCreateInfo, nullptr, &vkInstance))) {
       pDebugger->print("Created vulkan instance.");
    } else {
       pDebugger->print("Failed to create vulkan instance.", MESSAGE_TYPE::ERROR);
        MInstance::requestDestruction(errorCode);
    }
}

bool MVulkanContext::checkInstanceCreationErrors(const VkResult result) {
    pDebugger->print("Checking instance creation for errors . . .");
    if (result == VK_SUCCESS) {
       pDebugger->print("No errors found.");
        return true;
    } else {
        switch (result) {
            case VK_ERROR_INCOMPATIBLE_DRIVER:
                pDebugger->print("The Vulkan loader cannot find a driver that supports the requested API version or hardware requirements.",MESSAGE_TYPE::ERROR);
                errorCode = ErrorCode::INCOMPATIBLE_DRIVER;
                break;

            case VK_ERROR_OUT_OF_HOST_MEMORY:
                pDebugger->print("The host CPU system ran out of system memory.",MESSAGE_TYPE::ERROR);
                errorCode = ErrorCode::OUT_OF_MEMORY;
            break;

            case VK_ERROR_TOO_MANY_OBJECTS:
                pDebugger->print("Exceeded maximum number of active Vulkan instances supported by the driver.",MESSAGE_TYPE::ERROR);
                errorCode = ErrorCode::OUT_OF_MEMORY;
                break;

            case VK_ERROR_OUT_OF_DEVICE_MEMORY:
                pDebugger->print("The device ran out of memory allocated for system-level operations.",MESSAGE_TYPE::ERROR);
                break;

                case VK_ERROR_INITIALIZATION_FAILED:
               pDebugger->print("The Vulkan loader or implementation failed to initialize for an unspecified internal reason.",MESSAGE_TYPE::ERROR);
                errorCode = ErrorCode::INSTANCE_CREATION_FAILED;
                break;

            case VK_ERROR_LAYER_NOT_PRESENT:
                pDebugger->print("One or more requested instance validation layers could not be found.",MESSAGE_TYPE::ERROR);
                errorCode = ErrorCode::VULKAN_LAYER_MISSING;
                break;

            case VK_ERROR_EXTENSION_NOT_PRESENT:
                pDebugger->print("One or more requested instance extensions are not supported by the loader or installed drivers.",MESSAGE_TYPE::ERROR);
                errorCode = ErrorCode::VULKAN_EXTENSION_MISSING;
                break;

            default:
                pDebugger->print("An unknown critical error occurred within the Vulkan driver or loader.",MESSAGE_TYPE::ERROR);
                errorCode = ErrorCode::INSTANCE_CREATION_FAILED;
                break;
        }
        return false;
    }
}

void MVulkanContext::destroyVulkanInstance() {
    if (vkInstance != VK_NULL_HANDLE) {
        vkDestroyInstance(vkInstance, nullptr);
        vkInstance = VK_NULL_HANDLE;
        pDebugger->print("Destroyed vulkan instance.");
    }
}

//Device logic
void MVulkanContext::createLogicalDevice() {
pDebugger->print("Creating logical device . . .");
if (errorCode != ErrorCode::NO_ERRORS) {
    pDebugger->print("Cannot create logical device.",MESSAGE_TYPE::ERROR);
    return;
}else {
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    const std::set<uint32_t> uniqueQueueFamilies = { graphicsQueueFamilyIndex.value() };

    constexpr float queuePriority = 1.0f;
    for (const uint32_t queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.samplerAnisotropy = VK_TRUE;

    VkDeviceCreateInfo deviceCreateInfo{};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
    deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
    deviceCreateInfo.enabledExtensionCount = 0;
    deviceCreateInfo.ppEnabledExtensionNames = nullptr;
    deviceCreateInfo.enabledLayerCount = 0;

    if (vkCreateDevice(vkPhysicalDevice, &deviceCreateInfo, nullptr, &vkLogicalDevice) == VK_SUCCESS) {
        pDebugger->print("Successfully created logical device.");
    } else {
        pDebugger->print("Failed to create logical device!", MESSAGE_TYPE::ERROR);
        errorCode = ErrorCode::LOGICAL_DEVICE_CREATION_FAILED;
        MInstance::requestDestruction(errorCode);
    }
}
}

void MVulkanContext::destroyLogicalDevice() {
    if (vkLogicalDevice != VK_NULL_HANDLE) {
        pDebugger->print("Destroying logical device . . .");
        vkDestroyDevice(vkLogicalDevice, nullptr);
        vkLogicalDevice = VK_NULL_HANDLE;
       pDebugger->print("Destroyed logical device.");
    }
}

void MVulkanContext::selectPhysicalDevice() {
    pDebugger->print("Selecting a physical device.");
    if (errorCode != ErrorCode::NO_ERRORS) {
        pDebugger->print("Cannot select physical device",MESSAGE_TYPE::ERROR);
        return;
    }else {
        //Getting the data of all physical devices
        pDebugger->print("Getting all physical device . . .");

        uint32_t physicalDeviceCount = 0;
        vkEnumeratePhysicalDevices(vkInstance, &physicalDeviceCount, nullptr);

        if (physicalDeviceCount > 0) {
            pDebugger->print("Found " + std::to_string(physicalDeviceCount) + " physical device(s).");
        }else {
            pDebugger->print("Failed to find physical device with Vulkan support!",MESSAGE_TYPE::ERROR);
            errorCode = ErrorCode::PHYSICAL_DEVICE_NOT_FOUND;
            MInstance::requestDestruction(errorCode);
        }

        std::vector<VkPhysicalDevice> physicalDevicesList(physicalDeviceCount);
        vkEnumeratePhysicalDevices(vkInstance, &physicalDeviceCount, physicalDevicesList.data());

        //Selecting a suitable physical device
        vkPhysicalDevice = scorePhysicalDevices(physicalDevicesList);
        if (vkPhysicalDevice != VK_NULL_HANDLE) {
            vkGetPhysicalDeviceProperties(vkPhysicalDevice, &vkPhysicalDeviceProperties);
            vkGetPhysicalDeviceFeatures(vkPhysicalDevice, &vkPhysicalDeviceFeatures);
        }
    }
}

VkPhysicalDevice MVulkanContext::scorePhysicalDevices(const std::vector<VkPhysicalDevice> &scorablePhysicalDevicesList){
    pDebugger->print("Scoring a physical devices . . .");

    unsigned int maxPhysicalDeviceScore{0};
    VkPhysicalDevice suitablePhysicalDevice{VK_NULL_HANDLE};
    for (const auto physicalDevice : scorablePhysicalDevicesList) {
        unsigned int currentPhysicalDeviceScore{0};
    pDebugger->print("Scoring physical device . . .");

    //Physical device properties.
    pDebugger->print("Getting the physical device properties . . .");

    VkPhysicalDeviceProperties physicalDeviceProperties{};
    vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);
        const std::string deviceName = physicalDeviceProperties.deviceName;
    const VkPhysicalDeviceType physicalDeviceType = physicalDeviceProperties.deviceType;

    pDebugger->print("Got the physical device properties, name:" + deviceName);

    //Physical device features.
    pDebugger->print("Getting the physical device features . . .");

    VkPhysicalDeviceFeatures physicalDeviceFeatures{};
    vkGetPhysicalDeviceFeatures(physicalDevice, &physicalDeviceFeatures);

    pDebugger->print("Got the physical device features.");
        //Scoring logic

        //Mandatory properties and features
        {
            auto checkMandatoryPhysicalDevicePropertiesAndFeatures = [this](const std::string& attributeName ,const bool valueToCheck) {
               pDebugger->print("Checking " + attributeName + " support . . .");
                if (valueToCheck) {
                   pDebugger->print("The physical device supports " + attributeName + ".");
                    return  true;
                }else {
                    pDebugger->print("The device isn't suitable, doesn't support " + attributeName + ".",MESSAGE_TYPE::ERROR);
                    return false;
                }
            };
            pDebugger->print("Checking for mandatory features and properties . . .");
            findQueueFamilies(physicalDevice);
            if (!checkMandatoryPhysicalDevicePropertiesAndFeatures("graphics queue families",graphicsQueueFamilyIndex.has_value()) ||
            !checkMandatoryPhysicalDevicePropertiesAndFeatures("sampler anisotropy",physicalDeviceFeatures.samplerAnisotropy) ||
            !checkMandatoryPhysicalDevicePropertiesAndFeatures("independent blend",physicalDeviceFeatures.independentBlend) ||
            !checkMandatoryPhysicalDevicePropertiesAndFeatures("fillModeNonSolid",physicalDeviceFeatures.fillModeNonSolid) ||
            !checkMandatoryPhysicalDevicePropertiesAndFeatures("depth clamp",physicalDeviceFeatures.depthClamp) ||
            !checkMandatoryPhysicalDevicePropertiesAndFeatures("geometry shaders",physicalDeviceFeatures.geometryShader)) {
                currentPhysicalDeviceScore = 0;
            }

        }
        //Optional properties  and features
        {

            auto checkOptionalPhysicalDevicePropertiesAndFeatures = [this](const std::string& attributeName ,const bool valueToCheck, const unsigned int pointToAdd) {
               pDebugger->print("Checking " + attributeName + ". . .");
                if (valueToCheck) {
                    pDebugger->print("The physical device has " + attributeName + "(score:+" + std::to_string(pointToAdd) + ").");
                }
                return pointToAdd;
            };
           currentPhysicalDeviceScore = checkOptionalPhysicalDevicePropertiesAndFeatures("discrete GPU",physicalDeviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU,1000)+
            checkOptionalPhysicalDevicePropertiesAndFeatures("maxImageDimension2D",true,physicalDeviceProperties.limits.maxImageDimension2D/1000)+
                checkOptionalPhysicalDevicePropertiesAndFeatures("maxSamplerAnisotropy",true,physicalDeviceProperties.limits.maxSamplerAnisotropy * 10)+
                    checkOptionalPhysicalDevicePropertiesAndFeatures("maxBoundDescriptorSets",true,physicalDeviceProperties.limits.maxBoundDescriptorSets);
        }
        //Final scoring
        if (currentPhysicalDeviceScore > maxPhysicalDeviceScore) {
       pDebugger->print("Found a new physical device properties, score: " + std::to_string(maxPhysicalDeviceScore) + ",name: " + deviceName);
      maxPhysicalDeviceScore = currentPhysicalDeviceScore;
            suitablePhysicalDevice = physicalDevice;
    }else if (currentPhysicalDeviceScore < maxPhysicalDeviceScore){
        pDebugger->print("Physical device had a worst score: " + std::to_string(maxPhysicalDeviceScore) + ",name: " + deviceName,MESSAGE_TYPE::WARNING);
    };
};
    pDebugger->print("Scored all physical devices.");
    if (suitablePhysicalDevice == VK_NULL_HANDLE) {
       pDebugger->print("Couldn't find a suitable physical device.",MESSAGE_TYPE::ERROR);
        errorCode = ErrorCode::NO_SUITABLE_PHYSICAL_DEVICE;
        MInstance::requestDestruction(errorCode);
        return  VK_NULL_HANDLE;
    }else {
        pDebugger->print("Found a suitable physical device.");
        return suitablePhysicalDevice;
    }
};

void MVulkanContext::findQueueFamilies(const VkPhysicalDevice &physicalDevice) {
    pDebugger->print("Finding  the queue families . . .");
    uint32_t queueFamilyCount{0};
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

    for (uint32_t i = 0; i < queueFamilyCount; ++i) {
        // Check for graphics support
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
           graphicsQueueFamilyIndex = i;

        }

        if (graphicsQueueFamilyIndex.has_value()) {
            pDebugger->print("Found the graphics queue family: " + std::to_string(i) + ".");
            return;
        }
    }
    pDebugger->print("Couldn't find the graphics queue family.",MESSAGE_TYPE::ERROR);
}
