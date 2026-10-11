#include "../../../include/Magma/Window/windowHandler.h"

using namespace Magma;

/*=============================================================================
  Getting user OS
  =============================================================================*/
#ifdef __linux__
#include "../../../include/Magma/Window/linuxW.h"
#include <vulkan/vulkan_xlib.h>
std::vector<const char *> MWindowHandler::OSSpecificExtensions = {
    VK_KHR_XLIB_SURFACE_EXTENSION_NAME};
USER_OS MWindowHandler::userOS = LINUX;
#elif defined(_WIN32)
std::vector<const char *> MWindowHandler::OSSpecificExtensions = {};
USER_OS MWindowHandler::userOS = WIN_32_BIT;
#elif defined(_WIN64)
std::vector<const char *> MWindowHandler::OSSpecificExtensions = {};
USER_OS MWindowHandler::userOS = WIN_64_BIT;
#elif defined(__APPLE__) || defined(__MACH__)
std::vector<const char *> MWindowHandler::OSSpecificExtensions = {};
USER_OS MWindowHandler::userOS = MAC;
#else
std::vector<const char *> MWindowHandler::OSSpecificExtensions = {};
USER_OS MWindowHandler::userOS = UNSUPPORTED;
#endif

/*=============================================================================
  Explicit static values declaration
  =============================================================================*/
// Window creation logic
std::unique_ptr<MWindowHandler>
MWindowHandler::create(const std::string &appName, WINDOW_MODE windowMode,
                       const unsigned int windowSize[2], MDebugger &debugger) {
  debugger.print("Constructed window handler.");

  // OS dependent logic
  switch (userOS) {
  case LINUX:
    debugger.print("Using Linux window handler.");
    return std::make_unique<MLinuxHandler>(appName, windowMode, windowSize,
                                           debugger);
    break;
  default:
    throw std::runtime_error("The OS isn't supported.");
  };
};

/*=============================================================================
  Class init and destruction
  =============================================================================*/
MWindowHandler::MWindowHandler(
    const std::string& appName,
    const WINDOW_MODE windowMode,
    const unsigned int windowSize[2],
    MDebugger& debugger
)
    : mode(windowMode),
      windowTitle(appName),
      pDebugger(&debugger)
{
  size[0] = windowSize[0];
  size[1] = windowSize[1];
};

MWindowHandler::~MWindowHandler() {
  pDebugger->print("Destroying window handler.");

  pDebugger = nullptr;
};