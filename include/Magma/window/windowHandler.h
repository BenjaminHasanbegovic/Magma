#pragma once

#include "../Uitls/Debugger.h"
#include "../Uitls/ErrorCodes.h"
#include <memory>
#include <string>
#include <vector>
#include <vulkan/vulkan.h>

namespace Magma {

/*=============================================================================
  Essential class values
  =============================================================================*/
enum WINDOW_MODE : unsigned int {
  BORDERLESS = 0,
  FULL_SCREEN = 1,
  BORDERLESS_FULL_SCREEN = 2,
  OVERLAY = 3,
};

enum USER_OS : unsigned int {
  UNSUPPORTED = 0,
  LINUX = 1,
  WIN_32_BIT = 2,
  WIN_64_BIT = 3,
  MAC = 4,
};

/*=============================================================================
  Main class
  =============================================================================*/
class MWindowHandler {
  /* This class will be parent to any new OS handler implementation, it is just
   here to be overridden */
public:
  /*=============================================================================
    Parent creation
    =============================================================================*/
  static std::unique_ptr<MWindowHandler>
  create(const std::string &appName, WINDOW_MODE windowMode,
         const unsigned int windowSize[2], MDebugger &debugger);

  /*=============================================================================
    Class init and destruction
    =============================================================================*/
  MWindowHandler(const std::string &appName, WINDOW_MODE windowMode,
                 const unsigned int windowSize[2], MDebugger &debugger);
  virtual ~MWindowHandler();

  /*=============================================================================
    Window logic
    =============================================================================*/
  virtual void initWindow() = 0;
  virtual void createWindow() = 0;

  /*=============================================================================
    OS
    =============================================================================*/
  static USER_OS getUserOS() { return userOS; };
  static std::vector<const char *> getOSSpecificExtensions() {
    return OSSpecificExtensions;
  };

  /*=============================================================================
    Resizing logic
    =============================================================================*/
  virtual void getFramebufferSize(uint32_t &width, uint32_t &height) const = 0;
  bool framebufferResized{false};

  /*=============================================================================
    Vulkan logic
    =============================================================================*/
  virtual void createVulkanSurface(VkInstance instance) = 0;
  [[nodiscard]] VkSurfaceKHR getVkSurface() const { return *pSurface; };

  /*=============================================================================
    Event logic
    =============================================================================*/
  virtual void pollEvents(bool &runningState) = 0;
  virtual void extractEvents() = 0;

protected:
  /*=============================================================================
    Window info
    =============================================================================*/
  WINDOW_MODE mode;
  unsigned int size[2]{}; // [0] width & [1] height
  bool created{false};

  /*=============================================================================
    Magma
    =============================================================================*/
  const char *pAppName{nullptr};

  /*=============================================================================
    OS
    =============================================================================*/
  static USER_OS userOS;
  static std::vector<const char *> OSSpecificExtensions;

  /*=============================================================================
    Vulkan info
    =============================================================================*/
  VkSurfaceKHR *pSurface{VK_NULL_HANDLE};

  /*=============================================================================
    Error handling
    =============================================================================*/
  static ErrorCode errorCode;
  MDebugger *pDebugger{nullptr};
};
}; // namespace Magma