#include "../../../include/Magma/vulkan/VulkanContext.h"
#include "../../../include/Magma/Magma.h"
#include <set>
#include <xcb/xcb.h>
#include <vulkan/vulkan_xcb.h>

using namespace Magma;

     /* # =============================================================================
        #  Explicit static values declaration
        # ============================================================================= */
std::optional<uint32_t> MVulkanContext::graphicsQueueFamilyIndex = std::nullopt;
std::optional<uint32_t> MVulkanContext::presentQueueFamilyIndex = std::nullopt;
ErrorCode MVulkanContext::errorCode = ErrorCode::NO_ERRORS;
std::vector<const char*> MVulkanContext::enabledExtensions = {
    VK_KHR_SURFACE_EXTENSION_NAME
};

    /* # =============================================================================
       #  Class init and destruction
       # ============================================================================= */
MVulkanContext::MVulkanContext(const std::string& appName,MDebugger& debugger){
    //Initialize class variables
    pAppName = appName.c_str();
    pDebugger = &debugger;
};

MVulkanContext::~MVulkanContext() {
    pDebugger->print("Destroying vulkan context . . .");
    destroyLogicalDevice();
    destroyVulkanInstance();
    pDebugger->print("Destroyed vulkan context.");
};


  /* # =============================================================================
     #  Vulkan instance
     # ============================================================================= */
void MVulkanContext::createInstance() {
    pDebugger->print("Creating vulkan instance . . .");

    //Define data needed for creation info of Vulkan Instance
    pDebugger->print("Generating app info . . .");
    VkApplicationInfo appInfo {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.apiVersion = VK_API_VERSION_1_0;
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pApplicationName = pAppName;
    appInfo.pNext = nullptr;

    //Vulkan instance creation logic
   pDebugger->print("Generating instance create info . . .");
    VkInstanceCreateInfo instanceCreateInfo {};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pApplicationInfo = &appInfo;
    instanceCreateInfo.enabledLayerCount = 0;
    instanceCreateInfo.ppEnabledLayerNames = nullptr;
    instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(enabledExtensions.size());
    instanceCreateInfo.ppEnabledExtensionNames = enabledExtensions.data();
    instanceCreateInfo.pNext = nullptr;

    //Error handling
    if (checkInstanceCreationErrors(vkCreateInstance(&instanceCreateInfo, nullptr, &vkInstance))) {
       pDebugger->print("Created vulkan instance.");
    } else {
        throw std::runtime_error("Failed to create vulkan instance:" + errorCodeToString(errorCode));
    };
};

void MVulkanContext::destroyVulkanInstance() {
    if (vkInstance != VK_NULL_HANDLE) {
        pDebugger->print("Destroying vulkan instance . . .");

        vkDestroyInstance(vkInstance, nullptr);
        vkInstance = VK_NULL_HANDLE;

        pDebugger->print("Destroyed vulkan instance.");
    }
}

     /* # =============================================================================
        #  Os handling
        # ============================================================================= */
void MVulkanContext::setOSExtensions(const USER_OS OS,const std::vector<const char*>& OSExtensions) {
    if (OS != USER_OS::UNSUPPORTED) {
        enabledExtensions.insert(enabledExtensions.end(), OSExtensions.begin(), OSExtensions.end());
    }else {
        errorCode = ErrorCode::OS_NOT_SUPPORTED;
        throw std::runtime_error("Failed to set OSExtensions:" + errorCodeToString(errorCode));
    }
};

/* # =============================================================================
   #  Logical device
   # ============================================================================= */
void MVulkanContext::createLogicalDevice() {
pDebugger->print("Creating logical device . . .");

    //Values needed to create logical device
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    const std::set<uint32_t> uniqueQueueFamilies = { graphicsQueueFamilyIndex.value() };

    constexpr float queuePriority = 1.0f;

    VkPhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.samplerAnisotropy = VK_TRUE;

    //Getting the queue families
    for (const uint32_t queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    };

    //Logical device creation logic
    VkDeviceCreateInfo deviceCreateInfo{};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
    deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
    deviceCreateInfo.enabledExtensionCount = 0;
    deviceCreateInfo.ppEnabledExtensionNames = nullptr;
    deviceCreateInfo.enabledLayerCount = 0;

    //Error handling
    if (vkCreateDevice(vkPhysicalDevice, &deviceCreateInfo, nullptr, &vkLogicalDevice) == VK_SUCCESS) {
        pDebugger->print("Successfully created logical device.");
    } else {
        errorCode = ErrorCode::LOGICAL_DEVICE_CREATION_FAILED;
        throw std::runtime_error("Failed to crate logical device: " + errorCodeToString(errorCode));
    };
};

void MVulkanContext::destroyLogicalDevice() {
    if (vkLogicalDevice != VK_NULL_HANDLE) {
        pDebugger->print("Destroying logical device . . .");

        vkDestroyDevice(vkLogicalDevice, nullptr);
        vkLogicalDevice = VK_NULL_HANDLE;

       pDebugger->print("Destroyed logical device.");
    };
};


/* # =============================================================================
   #  Physical device
   # ============================================================================= */
void MVulkanContext::selectPhysicalDevice() {
    pDebugger->print("Selecting a physical device.");

        //Getting the data of all physical devices
        pDebugger->print("Getting all physical device . . .");

        uint32_t physicalDeviceCount = 0;
        vkEnumeratePhysicalDevices(vkInstance, &physicalDeviceCount, nullptr);

        if (physicalDeviceCount > 0) {
            pDebugger->print("Found " + std::to_string(physicalDeviceCount) + " physical device(s).");
        }else {
            errorCode = ErrorCode::NO_PHYSICAL_DEVICES_FOUND;
            throw std::runtime_error("Failed to find physical device with Vulkan support: " + errorCodeToString(errorCode));
        };

        std::vector<VkPhysicalDevice> physicalDevicesList(physicalDeviceCount);
        vkEnumeratePhysicalDevices(vkInstance, &physicalDeviceCount, physicalDevicesList.data());

        //Selecting a suitable physical device
        vkPhysicalDevice = scorePhysicalDevices(physicalDevicesList);
        if (vkPhysicalDevice != VK_NULL_HANDLE) {
            vkGetPhysicalDeviceProperties(vkPhysicalDevice, &vkPhysicalDeviceProperties);
            vkGetPhysicalDeviceFeatures(vkPhysicalDevice, &vkPhysicalDeviceFeatures);
        }else {
            throw std::runtime_error("Failed to find suitable physical device: " + errorCodeToString(errorCode));
        }
    };


VkPhysicalDevice MVulkanContext::scorePhysicalDevices(const std::vector<VkPhysicalDevice> &scorablePhysicalDevicesList) const {
    pDebugger->print("Scoring a physical devices . . .");

    //Values needed for scoring the physical device
    unsigned int maxPhysicalDeviceScore{0};
    VkPhysicalDevice suitablePhysicalDevice{VK_NULL_HANDLE};

    for (const auto physicalDevice : scorablePhysicalDevicesList) {
        unsigned int currentPhysicalDeviceScore{0};

        pDebugger->print("Scoring physical device . . .");

    //Physical device properties.
    pDebugger->print("Getting the physical device properties . . .");

        //Retrieving the properties of physical device
    VkPhysicalDeviceProperties physicalDeviceProperties{};
    vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);
        const std::string deviceName = physicalDeviceProperties.deviceName;
    const VkPhysicalDeviceType physicalDeviceType = physicalDeviceProperties.deviceType;

    pDebugger->print("Got the physical device properties, name: " + deviceName);

    //Physical device features.
    pDebugger->print("Getting the physical device features . . .");

        //Retrieving the features of physical device
    VkPhysicalDeviceFeatures physicalDeviceFeatures{};
    vkGetPhysicalDeviceFeatures(physicalDevice, &physicalDeviceFeatures);

    pDebugger->print("Got the physical device features.");

        //Scoring logic

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

            if (!checkMandatoryPhysicalDevicePropertiesAndFeatures("sampler anisotropy",physicalDeviceFeatures.samplerAnisotropy) ||
            !checkMandatoryPhysicalDevicePropertiesAndFeatures("independent blend",physicalDeviceFeatures.independentBlend) ||
            !checkMandatoryPhysicalDevicePropertiesAndFeatures("fillModeNonSolid",physicalDeviceFeatures.fillModeNonSolid) ||
            !checkMandatoryPhysicalDevicePropertiesAndFeatures("depth clamp",physicalDeviceFeatures.depthClamp) ||
            !checkMandatoryPhysicalDevicePropertiesAndFeatures("geometry shaders",physicalDeviceFeatures.geometryShader)) {
                currentPhysicalDeviceScore = 0;
            }

        }
        {
            auto checkOptionalPhysicalDevicePropertiesAndFeatures = [this](const std::string& attributeName ,const bool valueToCheck, const unsigned int pointToAdd) {
               pDebugger->print("Checking " + attributeName + ". . .");
                if (valueToCheck) {
                    pDebugger->print("The physical device has " + attributeName + "(score:+" + std::to_string(pointToAdd) + ").");
                    return pointToAdd;
                }else {
                    pDebugger->print("The physical device does not have: " + attributeName);
                    return static_cast<unsigned int>(0);
                }

            };
           currentPhysicalDeviceScore = checkOptionalPhysicalDevicePropertiesAndFeatures("discrete GPU",physicalDeviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU,1000)+
            checkOptionalPhysicalDevicePropertiesAndFeatures("maxImageDimension2D",true,physicalDeviceProperties.limits.maxImageDimension2D/1000)+
                checkOptionalPhysicalDevicePropertiesAndFeatures("maxSamplerAnisotropy",true,physicalDeviceProperties.limits.maxSamplerAnisotropy * 10)+
                    checkOptionalPhysicalDevicePropertiesAndFeatures("maxBoundDescriptorSets",true,physicalDeviceProperties.limits.maxBoundDescriptorSets);
        }

        //Compering scores
        if (currentPhysicalDeviceScore > maxPhysicalDeviceScore) {
       pDebugger->print("Found a new physical device properties, score: " + std::to_string(maxPhysicalDeviceScore) + ",name: " + deviceName);
      maxPhysicalDeviceScore = currentPhysicalDeviceScore;
            suitablePhysicalDevice = physicalDevice;
    }else if (currentPhysicalDeviceScore < maxPhysicalDeviceScore && maxPhysicalDeviceScore != 0){
        pDebugger->print("Physical device had a worst score: " + std::to_string(maxPhysicalDeviceScore) + ",name: " + deviceName,MESSAGE_TYPE::INFO);
    };
};

    //Final scoring
    pDebugger->print("Scored all physical devices.");

    if (suitablePhysicalDevice == VK_NULL_HANDLE) {
        errorCode = ErrorCode::NO_SUITABLE_PHYSICAL_DEVICE;
        return  VK_NULL_HANDLE;
    }else {
        pDebugger->print("Found a suitable physical device.");
        return suitablePhysicalDevice;
    };
};

void MVulkanContext::findMandatoryQueueFamilies() const {
    pDebugger->print("Finding the queue families . . .");

    uint32_t queueFamilyCount{0};
    vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice, &queueFamilyCount, queueFamilyProperties.data());

    for (uint32_t i = 0; i < queueFamilyCount; ++i) {
        // Check for graphics queue
        if (queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
           graphicsQueueFamilyIndex = i;
            pDebugger->print("Found the graphics queue family,index: " + std::to_string(i) + ".");
        };

        // Check for present queue
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(vkPhysicalDevice,i,VkSurface,&presentSupport);
        if (queueFamilyProperties[i].queueCount > 0 && presentSupport) {
            presentQueueFamilyIndex = i;
            pDebugger->print("Found the present queue family,index: " + std::to_string(i) + ".");
        };

        if (graphicsQueueFamilyIndex.has_value() && presentQueueFamilyIndex.has_value()) {
            pDebugger->print("Found all needed queue families.");
            break;
        }
    };

    //Error handling
    if (!graphicsQueueFamilyIndex.has_value()) {
        errorCode = ErrorCode::NO_GRAPHICAL_QUEUE_INDEX_FOUND;
        throw std::runtime_error("Couldn't find the graphics queue family: " + errorCodeToString(errorCode));
    }
    if (!presentQueueFamilyIndex.has_value()) {
        errorCode = ErrorCode::PRESENT_QUEUE_SUBMIT_FAILED;
        throw std::runtime_error("Couldn't find the present queue family: " + errorCodeToString(errorCode));
    };
};

     /* # =============================================================================
        #  Error handling
        # ============================================================================= */
bool MVulkanContext::checkInstanceCreationErrors(const VkResult result) const {
    pDebugger->print("Checking instance creation for errors . . .");
    //Error handling logic for Vulkan instance creation
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