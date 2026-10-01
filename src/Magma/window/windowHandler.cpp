#include "../../../include/Magma/window/windowHandler.h"
#include "../../../include/Magma/Magma.h"

using namespace Magma;
    /* # =============================================================================
       #  Getting user OS
       # ============================================================================= */
#ifdef __linux__
#include "../../../include/Magma/window/linuxW.h"
#include <xcb/xcb.h>
#define VK_USE_PLATFORM_XCB_KHR
#include <vulkan/vulkan.h>
std::vector<const char*> MWindowHandler::OSSpecificExtensions = {VK_KHR_XCB_SURFACE_EXTENSION_NAME};
USER_OS MWindowHandler::userOS = LINUX;
#elif defined(_WIN32)
std::vector<const char*> MWindowHandler::OSSpecificExtensions = {};
USER_OS MWindowHandler::userOS = WIN_32_BIT;
#elif defined(_WIN64)
std::vector<const char*> MWindowHandler::OSSpecificExtensions = {};
USER_OS MWindowHandler::userOS = WIN_64_BIT;
#elif defined(__APPLE__) || defined(__MACH__)
std::vector<const char*> MWindowHandler::OSSpecificExtensions = {};
USER_OS MWindowHandler::userOS = MAC;
#else
std::vector<const char*> MWindowHandler::OSSpecificExtensions = {};
USER_OS MWindowHandler::userOS = UNSUPPORTED;
#endif


     /* # =============================================================================
        #  Explicit static values declaration
        # ============================================================================= */
ErrorCode MWindowHandler::errorCode = ErrorCode::NO_ERRORS;
//Window creation logic
std::unique_ptr<MWindowHandler> MWindowHandler::create(const std::string& appName, WINDOW_MODE windowMode, const unsigned int windowSize[2], MDebugger& debugger) {
    debugger.print("Initializing window handler . . .");
//OS dependent logic
#if defined(__linux__)
    debugger.print("Using Linux window handler.");
    return std::make_unique<MLinuxHandler>(appName, windowMode, windowSize, debugger);

#elif defined(_WIN32) || defined(_WIN64)
    debugger.print("Using Windows window handler.");
     return std::make_unique<MWindowsHandler>(appName, windowMode, windowSize, debugger);
    return nullptr;

#elif defined(__APPLE__) || defined(__MACH__)
    debugger.print("Using MAC window handler.");
     return std::make_unique<MMacHandler>(appName, windowMode, windowSize, debugger);

#else
    debugger.print("The OS isn't supported.", MESSAGE_TYPE::ERROR);
    errorCode = ErrorCode::OS_NOT_SUPPORTED;
    MInstance::requestDestruction(errorCode);
    return nullptr;
#endif
};

/* # =============================================================================
   #  Class init and destruction
   # ============================================================================= */
MWindowHandler::MWindowHandler(const std::string& appName, WINDOW_MODE windowMode, const unsigned int windowSize[2], MDebugger& debugger)
    : appName(appName), mode(windowMode), pDebugger(&debugger) {
    size[0] = windowSize[0];
    size[1] = windowSize[1];
};

MWindowHandler::~MWindowHandler() {
    pDebugger->print("Destroying window handler . . .");
    pDebugger->print("Destroyed window handler.");
};