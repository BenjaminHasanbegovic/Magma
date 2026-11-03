#include "../../../include/Magma/Vulkan/VulkanContext.h"
#include "../../../include/Magma/Magma.h"
#include <set>
#include <unordered_map>
#include <xcb/xcb.h>

using namespace Magma;

/*=============================================================================
  Explicit static values declaration
  =============================================================================*/
// Queue families
std::optional<uint32_t> MVulkanContext::graphicsQueueFamilyIndex = std::nullopt;
std::optional<uint32_t> MVulkanContext::presentQueueFamilyIndex = std::nullopt;

// Extensions
std::vector<const char *> MVulkanContext::enabledExtensions = {
    VK_KHR_SURFACE_EXTENSION_NAME};
std::vector<const char *> MVulkanContext::requiredLogicalDeviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME};

//Features
std::vector<VkBool32 VkPhysicalDeviceFeatures::*> MVulkanContext::requiredFeatures{
  &VkPhysicalDeviceFeatures::samplerAnisotropy,
  &VkPhysicalDeviceFeatures::independentBlend,
  &VkPhysicalDeviceFeatures::fillModeNonSolid,
  &VkPhysicalDeviceFeatures::depthClamp,
  &VkPhysicalDeviceFeatures::geometryShader,

};
std::vector<VkBool32 VkPhysicalDeviceVulkan13Features::*> MVulkanContext::requiredFeatures13{
  &VkPhysicalDeviceVulkan13Features::dynamicRendering,
  &VkPhysicalDeviceVulkan13Features::synchronization2,
};

//Scoring
const std::unordered_map<VkPhysicalDeviceType, unsigned int>
MVulkanContext::deviceTypeScores = {
  {VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU,   1000},
  {VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU,  500},
  {VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU,     100},
  {VK_PHYSICAL_DEVICE_TYPE_CPU,              10},
};

const std::vector<MVulkanContext::ScoreRule>
MVulkanContext::physicalDeviceScoreRules = {
  {.name = "device type",
      .evaluate =
             [](const PhysicalDeviceCandidate &c) {
               const auto it = deviceTypeScores.find(c.properties.deviceType);
               return it != deviceTypeScores.end() ? it->second : 0u;
             }},

 {.name = "maxImageDimension2D",
     .evaluate =
             [](const PhysicalDeviceCandidate &c) {
               return c.properties.limits.maxImageDimension2D / 1000;
             }},

{.name = "maxSamplerAnisotropy",
    .evaluate =
             [](const PhysicalDeviceCandidate &c) {
               return static_cast<unsigned int>(
                          c.properties.limits.maxSamplerAnisotropy) *
                      10;
             }},

{.name = "maxBoundDescriptorSets",
    .evaluate =
             [](const PhysicalDeviceCandidate &c) {
               return c.properties.limits.maxBoundDescriptorSets;
             }},


{.name = "wideLines",
    .evaluate =
             [](const PhysicalDeviceCandidate &c) {
               return c.features.wideLines ? 50u : 0u;
             }},
};

/*=============================================================================
  Class init and destruction
  =============================================================================*/
MVulkanContext::MVulkanContext(const std::string &appName,
                               MDebugger &debugger) {
  // Initialize class variables
  pAppName = appName.c_str();
  pDebugger = &debugger;
  pDebugger->print("Constructed Vulkan context.");
};

MVulkanContext::~MVulkanContext() {
  pDebugger->print("Destroying Vulkan context.");
  destroyLogicalDevice();
  destroyInstance();
};

/*=============================================================================
  Vulkan instance
  =============================================================================*/
void MVulkanContext::createInstance() {
  pDebugger->print("Creating Vulkan instance . . .");

  // Define data needed for creation info of Vulkan Instance
  pDebugger->print("Generating app info . . .");
  VkApplicationInfo appInfo{};
  appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.apiVersion = VK_API_VERSION_1_3;
  appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.pApplicationName = pAppName;
  appInfo.pNext = nullptr;

  // Vulkan instance creation logic
  pDebugger->print("Generating instance create info . . .");
  VkInstanceCreateInfo instanceCreateInfo{};
  instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  instanceCreateInfo.pApplicationInfo = &appInfo;
  instanceCreateInfo.enabledLayerCount = 0;
  instanceCreateInfo.ppEnabledLayerNames = nullptr;
  instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(enabledExtensions.size());
  instanceCreateInfo.ppEnabledExtensionNames = enabledExtensions.data();
  instanceCreateInfo.pNext = nullptr;

  // Error handling
  checkInstanceCreationErrors(
      vkCreateInstance(&instanceCreateInfo, nullptr, &instance));

  pDebugger->print("Created Vulkan instance.");
};

void MVulkanContext::destroyInstance() {
  if (instance != VK_NULL_HANDLE) {
    pDebugger->print("Destroying Vulkan instance . . .");

    vkDestroyInstance(instance, nullptr);
    instance = VK_NULL_HANDLE;

    pDebugger->print("Destroyed Vulkan instance.");
  };
};

/*=============================================================================
  OS
  =============================================================================*/
void MVulkanContext::setOSExtensions(
    const unsigned int OS, const std::vector<const char *> &OSExtensions) {
  if (OS != USER_OS::UNSUPPORTED) {
    enabledExtensions.insert(enabledExtensions.end(), OSExtensions.begin(),OSExtensions.end());
  } else {
    throw std::runtime_error("Failed to set OSExtensions!");
  };
};

/*=============================================================================
  Logical device
  =============================================================================*/
void MVulkanContext::createLogicalDevice() {
  pDebugger->print("Creating logical device . . .");

  // Values needed to create logical device
  std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
  const std::set<uint32_t> uniqueQueueFamilies = {
      graphicsQueueFamilyIndex.value(), presentQueueFamilyIndex.value()};

  constexpr float queuePriority = 1.0f;

  //Setting features
  VkPhysicalDeviceFeatures coreFeatures{};
  coreFeatures.samplerAnisotropy = VK_TRUE;

  VkPhysicalDeviceVulkan13Features deviceFeatures13{};
  deviceFeatures13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
  deviceFeatures13.dynamicRendering = VK_TRUE;
  deviceFeatures13.synchronization2 = VK_TRUE;

  // Getting the queue families
  for (const uint32_t queueFamily : uniqueQueueFamilies) {
    VkDeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = queueFamily;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;
    queueCreateInfos.push_back(queueCreateInfo);
  };

  // Logical device creation logic
  VkDeviceCreateInfo deviceCreateInfo{};
  deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
  deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
  deviceCreateInfo.pEnabledFeatures = &coreFeatures;
  deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(requiredLogicalDeviceExtensions.size());
  deviceCreateInfo.ppEnabledExtensionNames = requiredLogicalDeviceExtensions.data();
  deviceCreateInfo.enabledLayerCount = 0;
  deviceCreateInfo.pNext = &deviceFeatures13;

  // Error handling
  checkLogicalDeviceExtensionsSupport();

  if (vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr,
                     &logicalDevice) == VK_SUCCESS) {
    pDebugger->print("Successfully created logical device.");
  } else {
    throw std::runtime_error("Failed to crate logical device!");
  };

  vkGetDeviceQueue(logicalDevice, graphicsQueueFamilyIndex.value(), 0, &graphicsQueue);
  vkGetDeviceQueue(logicalDevice, presentQueueFamilyIndex.value(), 0, &presentQueue);
};

void MVulkanContext::destroyLogicalDevice() {
  if (logicalDevice != VK_NULL_HANDLE) {
    pDebugger->print("Destroying logical device . . .");

    vkDestroyDevice(logicalDevice, nullptr);
    logicalDevice = VK_NULL_HANDLE;

    pDebugger->print("Destroyed logical device.");
  };
};

void MVulkanContext::checkLogicalDeviceExtensionsSupport() const {
  pDebugger->print("Checking physical device extension support . . .");

  uint32_t extensionCount{0};
  vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount,
                                       nullptr);

  std::vector<VkExtensionProperties> availableExtensions(extensionCount);
  vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount,
                                       availableExtensions.data());

  std::set<std::string> requiredExtensions(requiredLogicalDeviceExtensions.begin(),
                                           requiredLogicalDeviceExtensions.end());

  for (const auto &[extensionName, specVersion] : availableExtensions) {
    requiredExtensions.erase(extensionName);
  };

  if (requiredExtensions.empty()) {
    pDebugger->print("All required device extensions are supported.");
  } else {
    throw std::runtime_error(
        "Missing required device extensions!");
  };
};

/*=============================================================================
  Physical device
  =============================================================================*/
void MVulkanContext::selectPhysicalDevice() {
  pDebugger->print("Selecting a physical device.");

  uint32_t physicalDeviceCount{0};
  vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr);

  if (physicalDeviceCount > 0) {
    pDebugger->print("Found " + std::to_string(physicalDeviceCount) +
                     " physical device(s).");
  } else {
    throw std::runtime_error("Failed to find physical device with Vulkan support!");
  };

  std::vector<VkPhysicalDevice> physicalDevicesList(physicalDeviceCount);
  vkEnumeratePhysicalDevices(instance, &physicalDeviceCount,
                             physicalDevicesList.data());

  physicalDevice = scorePhysicalDevices(physicalDevicesList);
  vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

  VkPhysicalDeviceVulkan13Features deviceFeatures13{};
  deviceFeatures13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;

  VkPhysicalDeviceFeatures2 deviceFeatures2{};
  deviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
  deviceFeatures2.pNext = &deviceFeatures13;

  physicalDeviceFeatures = deviceFeatures2.features;
  physicalDeviceFeatures13 = deviceFeatures13;

};

VkPhysicalDevice MVulkanContext::scorePhysicalDevices(
    const std::vector<VkPhysicalDevice> &scorablePhysicalDevicesList) const {
  pDebugger->print("Scoring a physical devices . . .");

  unsigned int maxPhysicalDeviceScore{0};
  VkPhysicalDevice suitablePhysicalDevice{VK_NULL_HANDLE};

  for (const auto physicalDeviceCandidate : scorablePhysicalDevicesList) {
    pDebugger->print("Scoring physical device . . .");

    unsigned int currentPhysicalDeviceScore{0};
    //Properties.
    pDebugger->print("Getting the physical device properties . . .");

    PhysicalDeviceCandidate candidate;

    VkPhysicalDeviceProperties candidatePhysicalDeviceProperties{};
    vkGetPhysicalDeviceProperties(physicalDeviceCandidate,
                                  &candidatePhysicalDeviceProperties);
    candidate.properties = candidatePhysicalDeviceProperties;
    const std::string deviceName = candidatePhysicalDeviceProperties.deviceName;
    const VkPhysicalDeviceType physicalDeviceType = candidatePhysicalDeviceProperties.deviceType;

    pDebugger->print("Got the physical device properties, name: " + deviceName);

    // Physical device features.
    pDebugger->print("Getting the physical device features . . .");

    // Retrieving the features of physical device
    VkPhysicalDeviceVulkan13Features candidateFeatures13{};
    candidateFeatures13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;

    VkPhysicalDeviceFeatures2 candidateFeatures2{};
    candidateFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    candidateFeatures2.pNext = &candidateFeatures13;

    vkGetPhysicalDeviceFeatures2(physicalDeviceCandidate, &candidateFeatures2);
    candidate.features = candidateFeatures2.features;

    pDebugger->print("Got the physical device features.");

    // Scoring logic
    pDebugger->print("Checking for required features . . .");

    if (!checkRequiredPhysicalDeviceFeaturesSupport(candidateFeatures2.features, candidateFeatures13)) {
      pDebugger->print("Missing required features, skipping: " + deviceName, MESSAGE_TYPE::WARNING);
      continue;
    };

    pDebugger->print("Checked for required features.");

    for (const auto &rule : physicalDeviceScoreRules) {
      const unsigned int points = rule.evaluate(candidate);
      currentPhysicalDeviceScore += points;

      if (points > 0) {
        pDebugger->print("Rule '" + rule.name + "' gave +" + std::to_string(points) + ".");
      } else {
        pDebugger->print("Rule '" + rule.name + "' gave no points.");
      };
    };

    pDebugger->print("Checked physical device properties.");

    // Compering scores
    if (currentPhysicalDeviceScore > maxPhysicalDeviceScore) {
      maxPhysicalDeviceScore = currentPhysicalDeviceScore;
      suitablePhysicalDevice = physicalDeviceCandidate;
      pDebugger->print("Found a new physical device properties, score: " +
                       std::to_string(maxPhysicalDeviceScore) +
                       ", name: " + deviceName);
    } else if (currentPhysicalDeviceScore < maxPhysicalDeviceScore &&
               maxPhysicalDeviceScore != 0) {
      pDebugger->print("Physical device had a worst score: " +
                           std::to_string(maxPhysicalDeviceScore) +
                           ",name: " + deviceName,MESSAGE_TYPE::WARNING);
    };
  };

  // Final scoring
  pDebugger->print("Scored all physical devices.");

  if (suitablePhysicalDevice == VK_NULL_HANDLE) {
    throw std::runtime_error("Could not find a suitable physical device!");
    return VK_NULL_HANDLE;
  } else {
    pDebugger->print("Found a suitable physical device.");
    return suitablePhysicalDevice;
  };
};

bool MVulkanContext::checkRequiredPhysicalDeviceFeaturesSupport(
    const VkPhysicalDeviceFeatures &supportedCoreFeatures,
    const VkPhysicalDeviceVulkan13Features &supportedFeatures13) {
  for (const auto feature : requiredFeatures) {
    if (!(supportedCoreFeatures.*feature)) return false;
  };

  for (const auto feature : requiredFeatures13) {
    if (!(supportedFeatures13.*feature)) return false;
  };

  return true;
};

/*=============================================================================
  Queue families
  =============================================================================*/
void MVulkanContext::findMandatoryQueueFamilyIndexes() const {
  pDebugger->print("Finding the queue family indexes . . .");

  uint32_t queueFamilyCount{0};

  vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount,
                                           nullptr);
std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount,
                                           queueFamilyProperties.data());

  for (uint32_t i = 0; i < queueFamilyCount; ++i) {
    // Check for graphics queue
    if (queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      graphicsQueueFamilyIndex = i;

      pDebugger->print(
          "Found the graphics queue family,index: " + std::to_string(i) + ".");
    };

    // Check for present queue
    VkBool32 presentSupport = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, pSurface,
                                         &presentSupport);
    if (queueFamilyProperties[i].queueCount > 0 && presentSupport) {
      presentQueueFamilyIndex = i;
      pDebugger->print(
          "Found the present queue family,index: " + std::to_string(i) + ".");
    };

    if (graphicsQueueFamilyIndex.has_value() &&
        presentQueueFamilyIndex.has_value()) {
      pDebugger->print("Found all needed queue families.");
      break;
    }
  };

  // Error handling
  if (!graphicsQueueFamilyIndex.has_value()) {
    throw std::runtime_error(
        "Couldn't find the graphics queue family!");
  }

  if (!presentQueueFamilyIndex.has_value()) {
    throw std::runtime_error("Couldn't find the present queue family!");
  };
};

/*=============================================================================
  Error handling
  =============================================================================*/
void MVulkanContext::checkInstanceCreationErrors(const VkResult result) const {
  pDebugger->print("Checking instance creation for errors . . .");
  // Error handling logic for Vulkan instance creation
  if (result == VK_SUCCESS) {
    pDebugger->print("No errors found.");
  } else {
    switch (result) {
    case VK_ERROR_INCOMPATIBLE_DRIVER:
      pDebugger->print("The Vulkan loader cannot find a driver that supports "
                       "the requested API version or hardware requirements.",
                       MESSAGE_TYPE::ERROR);

      break;
    case VK_ERROR_OUT_OF_HOST_MEMORY:
      pDebugger->print("The host CPU system ran out of system memory.",
                       MESSAGE_TYPE::ERROR);

      break;

    case VK_ERROR_TOO_MANY_OBJECTS:
      pDebugger->print("Exceeded maximum number of active Vulkan instances "
                       "supported by the driver.",
                       MESSAGE_TYPE::ERROR);

      break;

    case VK_ERROR_OUT_OF_DEVICE_MEMORY:
      pDebugger->print(
          "The device ran out of memory allocated for system-level operations.",
          MESSAGE_TYPE::ERROR);
      break;

    case VK_ERROR_INITIALIZATION_FAILED:
      pDebugger->print("The Vulkan loader or implementation failed to "
                       "initialize for an unspecified internal reason.",
                       MESSAGE_TYPE::ERROR);

      break;

    case VK_ERROR_LAYER_NOT_PRESENT:
      pDebugger->print("One or more requested instance validation layers could "
                       "not be found.",
                       MESSAGE_TYPE::ERROR);
      break;

    case VK_ERROR_EXTENSION_NOT_PRESENT:
      pDebugger->print("One or more requested instance extensions are not "
                       "supported by the loader or installed drivers.",
                       MESSAGE_TYPE::ERROR);
      break;

    default:
      pDebugger->print("An unknown critical error occurred within the Vulkan "
                       "driver or loader.",
                       MESSAGE_TYPE::ERROR);
      break;
    };
  };
};