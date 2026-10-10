#pragma once

#include "../Utils/Debugger.h"
#include "../Utils/ErrorCodes.h"

#include <optional>
#include <string>
#include <vector>
#include <vulkan/vulkan.h>

namespace Magma {
class MVulkanContext {
public:

  /*=============================================================================
    Class init and destruction
    =============================================================================*/
  MVulkanContext(const std::string &appName, MDebugger &debugger);
  ~MVulkanContext();

  /*=============================================================================
    Vulkan instance
    =============================================================================*/
  void createInstance();

  /*=============================================================================
    Logical device
    =============================================================================*/
  void createLogicalDevice();

  /*=============================================================================
    Physical device
    =============================================================================*/
  void selectPhysicalDevice();

  /*=============================================================================
    OS
    =============================================================================*/
  static void setOSExtensions(unsigned int OS,
                              const std::vector<const char *> &OSExtensions);

  /*=============================================================================
    Vulkan surface
    =============================================================================*/
  void setSurface(const VkSurfaceKHR &surface) { pSurface = surface; };

  /*=============================================================================
    Queue families
    =============================================================================*/
  void findMandatoryQueueFamilies() const;

  /*=============================================================================
    Getters
    =============================================================================*/
  [[nodiscard]] VkPhysicalDevice getPhysicalDevice() const {return physicalDevice;};
  [[nodiscard]] VkDevice getLogicalDevice() const { return logicalDevice; };
  [[nodiscard]] VkInstance getInstance() const { return instance; };
  static uint32_t getGraphicsQueueFamilyIndex() { return graphicsQueueFamilyIndex.value(); };

private:
  /*=============================================================================
    Magma instance
    =============================================================================*/
  const char *pAppName{nullptr};
  MDebugger *pDebugger{nullptr};

  /*=============================================================================
    OS
    =============================================================================*/
  static std::vector<const char *> enabledExtensions;

  /*=============================================================================
    Vulkan instance
    =============================================================================*/
  VkInstance instance = VK_NULL_HANDLE;
  VkSurfaceKHR pSurface = VK_NULL_HANDLE;
  void destroyInstance();

  /*=============================================================================
    Logical device
    =============================================================================*/
  VkDevice logicalDevice = VK_NULL_HANDLE;
  void destroyLogicalDevice();
  static std::vector<const char *> logicalDeviceExtensions;

  /*=============================================================================
    Physical device
    =============================================================================*/
  VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
  VkPhysicalDeviceProperties physicalDeviceProperties{};
  VkPhysicalDeviceFeatures physicalDeviceFeatures{};
  [[nodiscard]] VkPhysicalDevice scorePhysicalDevices(
      const std::vector<VkPhysicalDevice> &scorablePhysicalDevicesList) const;

  /*=============================================================================
    Queue families
    =============================================================================*/
  static std::optional<uint32_t> graphicsQueueFamilyIndex;
  static std::optional<uint32_t> presentQueueFamilyIndex;

  /*=============================================================================
    Error handling
    =============================================================================*/
  void checkInstanceCreationErrors(VkResult result) const;
  void checkVkLogicalDeviceExtensionsSupport() const;
};
}; // namespace Magma
