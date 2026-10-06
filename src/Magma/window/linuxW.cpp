#include "../../../include/Magma/window/linuxW.h"

using namespace Magma;

/*=============================================================================
  Class init and destruction
  =============================================================================*/
MLinuxHandler::MLinuxHandler(const std::string &appName,
                             const WINDOW_MODE windowMode,
                             const unsigned int windowSize[2],
                             MDebugger &debugger)
    : MWindowHandler(appName, windowMode, windowSize, debugger) {
  pDebugger->print("Constructed Linux X11 window handler.");
};

MLinuxHandler::~MLinuxHandler() {
  if (display != nullptr) {
    pDebugger->print("Destroying Linux X11 window . . .");
    XCloseDisplay(display);
    pDebugger->print("Destroyed Linux X11 window.");
  };
};

/*=============================================================================
  Window logic
  =============================================================================*/
void MLinuxHandler::initWindow() {
  pDebugger->print("Initializing X11 Display . . .");

  // Display init &  it's error handling
  display = XOpenDisplay(nullptr);
  if (display == nullptr) {
    errorCode = ErrorCode::OS_NOT_SUPPORTED;
    throw std::runtime_error("Failed to open X11 display: " +
                             errorCodeToString(errorCode));
    return;
  };

  // Screen init
  screen = DefaultScreen(display);
  backgroundColor = BlackPixel(display, screen);

  // Creating the window
  pDebugger->print("Creating a simple window.");
  window =
      XCreateSimpleWindow(display, DefaultRootWindow(display), 0, 0, size[0],
                          size[1], 0, backgroundColor, backgroundColor);

  // Getting the windows properties
  pDebugger->print("Setting window properties.");
  extractEvents();

  pDebugger->print("X11 Display initialized successfully.");
};

void MLinuxHandler::createWindow() {
  pDebugger->print("Creating Linux X11 window . . .");

  XSetStandardProperties(display, window, pAppName, pAppName, None, nullptr, 0,
                         nullptr);
  XMapWindow(display, window);
  XFlush(display);

  created = true;

  pDebugger->print("Created the Linux X11 window.");
};

void MLinuxHandler::getFramebufferSize(uint32_t &width,
                                       uint32_t &height) const {
  XWindowAttributes windowAttributes;
  if (XGetWindowAttributes(display, window, &windowAttributes)) {
    width = static_cast<uint32_t>(windowAttributes.width);
    height = static_cast<uint32_t>(windowAttributes.height);
  } else {
    width = size[0];
    height = size[1];
  };
};

/*=============================================================================
  Event logic
  =============================================================================*/
void MLinuxHandler::extractEvents() {
  pDebugger->print("Extracting needed window events.");
  XSelectInput(
      display, window,
      ExposureMask | // Tells the renderer that the window surface needs to be redrawn.
          KeyPressMask |    // Keyboard key is pushed down.
          KeyReleaseMask |  // Key is let go.
          ButtonPressMask | // Event when a mouse button (left, right, middle, or scroll wheel step) is clicked down.
          ButtonReleaseMask | // Event when a mouse button is released.
          PointerMotionMask | // Event continuously as long as the mouse cursor moves within the window bounds.
          StructureNotifyMask | ConfigureNotify | // Events when the window changes size, position, or stacking order.
          FocusChangeMask // Events when the user switches to or away from the app.
  );

  // Exit logic
  pDebugger->print("Getting delete message.");
  wmDeleteMessage = XInternAtom(display, "WM_DELETE_WINDOW", False);
  XSetWMProtocols(display, window, &wmDeleteMessage, 1);
};

void MLinuxHandler::pollEvents(bool &runningState) {
  pDebugger->print("Polling events . . .");

  // Essential event polling values
  XEvent event;
  XNextEvent(display, &event);

  if (event.type == ClientMessage) {
    pDebugger->print("Client message received.");

    {
      // Quiting the app handle
      if (static_cast<Atom>(event.xclient.data.l[0]) == wmDeleteMessage) {
        pDebugger->print("Window close requested.");
        runningState = false;
      };
    }

  } else if (event.type == ConfigureNotify) {
    pDebugger->print("Configure notify received.");
    {
      // Resize handle
      const auto newWidth = static_cast<uint32_t>(event.xconfigure.width);
      const auto newHeight = static_cast<uint32_t>(event.xconfigure.height);

      if (newWidth != size[0] || newHeight != size[1]) {
        size[0] = newWidth;
        size[1] = newHeight;
        framebufferResized = true;
      };
    };
  };
};

/*=============================================================================
  Vulkan logic
  =============================================================================*/
void MLinuxHandler::createVulkanSurface(VkInstance instance) {
  pDebugger->print("Creating Vulkan Xlib surface . . .");

  VkXlibSurfaceCreateInfoKHR createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
  createInfo.dpy = display;
  createInfo.window = window;

  if (vkCreateXlibSurfaceKHR(instance, &createInfo, nullptr, &this->pSurface) !=
      VK_SUCCESS) {
    errorCode = ErrorCode::SURFACE_CREATION_FAILED;
    throw std::runtime_error("Failed to create Vulkan X11 surface!");
  };

  pDebugger->print("Created Vulkan Xlib surface.");
};
