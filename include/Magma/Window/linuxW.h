#pragma once

#define VK_USE_PLATFORM_XLIB_KHR
#include "windowHandler.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <cstring>

namespace Magma {
class MLinuxHandler : public MWindowHandler {
public:
  /*=============================================================================
    Class init and destruction
    =============================================================================*/
  MLinuxHandler(const std::string &appName, WINDOW_MODE windowMode,
                const unsigned int windowSize[2], MDebugger &debugger);
  ~MLinuxHandler() override;

  /*=============================================================================
    Window logic
    =============================================================================*/
  void initWindow() override;
  void createWindow() override;
  void getFramebufferSize(uint32_t &width, uint32_t &height) const override;

  /*=============================================================================
    Event logic
    =============================================================================*/
  void pollEvents(bool &runningState) override;
  void extractEvents() override;

  /*=============================================================================
    Window creation data
    =============================================================================*/
  // Getters
  [[nodiscard]] Display *getX11Display() const { return display; }
  [[nodiscard]] Window getX11Window() const { return window; }
  [[nodiscard]] Atom getWmDeleteMessage() const { return wmDeleteMessage; }

  /*=============================================================================
    Vulkan logic
    =============================================================================*/
  void createSurface(VkInstance instance) override;

private:
  /*=============================================================================
    Window creation data
    =============================================================================*/
  Display *display{nullptr};
  unsigned long int screen{0};
  Window window{};
  Atom wmDeleteMessage{0};

  // Styling
  unsigned long backgroundColor{};
};
}; // namespace Magma