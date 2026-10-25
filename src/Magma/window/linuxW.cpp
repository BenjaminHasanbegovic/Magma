#include "../../../include/Magma/Window/linuxW.h"

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
    throw std::runtime_error("Failed to open X11 display!");
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

  extractEvents();
  pDebugger->print("X11 Display initialized successfully.");
};

void MLinuxHandler::createWindow() {
  pDebugger->print("Creating Linux X11 window . . .");

  XStoreName(display, window, windowTitle.c_str());
  const Atom netWmName =
      XInternAtom(display, "_NET_WM_NAME", False);

  const Atom utf8String =
      XInternAtom(display, "UTF8_STRING", False);

  if (netWmName != None && utf8String != None) {
    XChangeProperty(
        display,
        window,
        netWmName,
        utf8String,
        8,
        PropModeReplace,
        reinterpret_cast<const unsigned char*>(
            windowTitle.c_str()
        ),
        static_cast<int>(windowTitle.size())
    );
  }

  XFlush(display);
  XMapWindow(display, window);
  XFlush(display);
  created = true;

  setWindowMode(MWindowHandler::windowMode);

  pDebugger->print("Created the Linux X11 window.");
}

void MLinuxHandler::setWindowMode(WINDOW_MODE newWindowMode) {
  MWindowHandler::windowMode = newWindowMode;

  const Atom stateAtom = XInternAtom(display, "_NET_WM_STATE", False);
  const Atom fullscreenAtom = XInternAtom(display, "_NET_WM_STATE_FULLSCREEN", False);

  event.type = ClientMessage;
  event.xclient.window = window;
  event.xclient.message_type = stateAtom;
  event.xclient.format = 32;


  if (MWindowHandler::windowMode == WINDOW_MODE::FULLSCREEN) {
    pDebugger->print("Setting fullscreen window mode.");
    event.xclient.data.l[0] = 1;
  } else {
    pDebugger->print("Setting windowed mode.");
    event.xclient.data.l[0] = 0;
  }

  event.xclient.data.l[1] = fullscreenAtom;
  event.xclient.data.l[2] = 0;
  event.xclient.data.l[3] = 1;
  event.xclient.data.l[4] = 0;

  XSendEvent(display, DefaultRootWindow(display), False,
             SubstructureNotifyMask | SubstructureRedirectMask, &event);

  XFlush(display);
}

void MLinuxHandler::changeCursorStyle(const CURSOR_STYLE newCursorStyle) {
  pDebugger->print("Changing cursor style . . .");
  Cursor cursorStyle = 0;

  switch (newCursorStyle) {
  case CURSOR_STYLE::POINTER:
    cursorStyle = XCreateFontCursor(display, XC_left_ptr);
    pDebugger->print("Changed cursor style to POINTER.");
    break;

  case CURSOR_STYLE::LOADING:
    cursorStyle = XCreateFontCursor(display, XC_watch);
    pDebugger->print("Changed cursor style to LOADING.");
    break;

  case CURSOR_STYLE::PRESS:
    cursorStyle = XCreateFontCursor(display, XC_hand1);
    pDebugger->print("Changed cursor style to PRESS.");
    break;

  case CURSOR_STYLE::TYPE:
    cursorStyle = XCreateFontCursor(display, XC_xterm);
    pDebugger->print("Changed cursor style to TYPE (Text I-beam).");
    break;
  }

  if (cursorStyle) {
    XDefineCursor(display, window, cursorStyle);
    XFlush(display);
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
  XNextEvent(display, &event);
  if (event.type == ConfigureNotify) {
    pDebugger->print("Configure notify received.");
    {

      if (event.xconfigure.width != size[0] || event.xconfigure.height != size[1]) {
        size[0] = event.xconfigure.width;
        size[1] = event.xconfigure.height;
        resized = true;
      };
    };
  }else if (event.type == ClientMessage) {
    pDebugger->print("Client message received.");
    {
      // Quiting the app handle
      if (static_cast<Atom>(event.xclient.data.l[0]) == wmDeleteMessage) {
        pDebugger->print("Window close requested.");
        runningState = false;
      }
    };
  }
  else if (event.type == ButtonPress) {

  };
};

/*=============================================================================
  Vulkan logic
  =============================================================================*/
void MLinuxHandler::createSurface(VkInstance instance) {
  pDebugger->print("Creating Vulkan Xlib surface . . .");

  VkXlibSurfaceCreateInfoKHR createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
  createInfo.dpy = display;
  createInfo.window = window;

  if (vkCreateXlibSurfaceKHR(instance, &createInfo, nullptr, &this->surface) !=
      VK_SUCCESS) {
    throw std::runtime_error("Failed to create Vulkan X11 surface!");
  };
  pDebugger->print("Created Vulkan Xlib surface.");
};
