#include "../../../include/Magma/window/windowHandler.h"
#include "../../../include/Magma/Magma.h"

#ifdef __linux__
    #include "../../../include/Magma/window/linuxW.h"
#elif defined(_WIN32) || defined(_WIN64)
    // #include "../../../include/Magma/window/windowsW.h"
#endif

using namespace Magma;

ErrorCode MWindowHandler::errorCode = ErrorCode::NO_ERRORS;


std::unique_ptr<MWindowHandler> MWindowHandler::create(const std::string& appName, WINDOW_MODE windowMode, const unsigned int windowSize[2], MDebugger& debugger) {
    debugger.print("Initializing window handler . . .");

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
}

MWindowHandler::MWindowHandler(const std::string& appName, WINDOW_MODE windowMode, const unsigned int windowSize[2], MDebugger& debugger)
    : appName(appName), mode(windowMode), pDebugger(&debugger) {
    size[0] = windowSize[0];
    size[1] = windowSize[1];
}

MWindowHandler::~MWindowHandler() {
    pDebugger->print("Destroying window handler . . .");
    pDebugger->print("Destroyed window handler.");
}