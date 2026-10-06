#pragma once

#include "../Uitls/Debugger.h"
#include "../Uitls/ErrorCodes.h"

#include <optional>
#include <string>
#include <vector>
#include <vulkan/vulkan.h>

namespace Magma {

/*=============================================================================
  External classes definition
  =============================================================================*/
class MDebugger;

/*=============================================================================
   Main class
  =============================================================================*/
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
  [[nodiscard]] VkInstance getInstance() const { return instance; };

  /*=============================================================================
    Logical device
    =============================================================================*/
  void createLogicalDevice();
  [[nodiscard]] VkDevice getLogicalDevice() const { return logicalDevice; };

  /*=============================================================================
    Physical device
    =============================================================================*/
  void selectPhysicalDevice();
  [[nodiscard]] VkPhysicalDevice getPhysicalDeviceDevice() const {
    return physicalDevice;
  };

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
