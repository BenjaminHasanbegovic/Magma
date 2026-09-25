#include "../../../include/Magma/window/linuxW.h"

using namespace Magma;

MLinuxHandler::MLinuxHandler(const std::string& appName, WINDOW_MODE windowMode, const unsigned int windowSize[2], MDebugger& debugger)
    : MWindowHandler(appName, windowMode, windowSize, debugger) {

 initWindow();
}

MLinuxHandler::~MLinuxHandler() {
 pDebugger->print("Destroying Linux X11 window . . .");
 if (display != nullptr) {
  XCloseDisplay(display);
 }
 pDebugger->print("Destroyed Linux X11 window.");
}

void MLinuxHandler::initWindow() {
 pDebugger->print("Initializing X11 Display . . .");

 display = XOpenDisplay(nullptr);
 if (display == nullptr) {
  MDebugger::print("Failed to open X11 display.", MESSAGE_TYPE::ERROR);
  errorCode = ErrorCode::OS_NOT_SUPPORTED;
  MInstance::requestDestruction(errorCode);
  return;
 }

 screen = DefaultScreen(display);
 backgroundColor = BlackPixel(display, screen);

MDebugger::print("Getting needed window data.");
 window = XCreateSimpleWindow(
     display,
     DefaultRootWindow(display),
     0, 0,
     size[0], size[1],
     0,
     backgroundColor,
     backgroundColor
 );

 MDebugger::print("Setting window properties.");
 XSetStandardProperties(display, window, appName.c_str(), appName.c_str(), None, nullptr, 0, nullptr);

 MDebugger::print("Getting needed window input data.");
 XSelectInput(display, window,
     ExposureMask        |
     KeyPressMask        |
     KeyReleaseMask      |
     ButtonPressMask     |
     ButtonReleaseMask   |
     PointerMotionMask   |
     StructureNotifyMask |
     FocusChangeMask
 );

 MDebugger::print("Getting delete message.");
 wmDeleteMessage = XInternAtom(display, "WM_DELETE_WINDOW", False);
 XSetWMProtocols(display, window, &wmDeleteMessage, 1);

 XMapWindow(display, window);
 XFlush(display);

 MDebugger::print("X11 Display initialized successfully.");
}

void MLinuxHandler::pollEvents() {
 MDebugger::print("Polling events . . .");

 if (!display) {
  MDebugger::print("Couldn't poll events . . .", MESSAGE_TYPE::ERROR);
  errorCode = ErrorCode::INVALID_ARGUMENT;
  MInstance::requestDestruction(errorCode);
 }else {
  XEvent event;
  while (XPending(display) > 0) {
   XNextEvent(display,&event);
   if (event.type == ClientMessage) {
    if (static_cast<Atom>(event.xclient.data.l[0]) == wmDeleteMessage) {
    MDebugger::print("Window close requested.");
     errorCode = ErrorCode::NO_ERRORS;
     MInstance::requestDestruction(errorCode);
    }
   }
  }
 }
}