#pragma once

#include "Utils/Debugger.h"
#include "Vulkan/VulkanContext.h"
#include "Window/windowHandler.h"
#include "Vulkan/Renderer.h"
#include <string>
#include <chrono>

namespace Magma {
class MInstance {
public:
  /*=============================================================================
    Class init and destruction
    =============================================================================*/
  explicit MInstance(std::string appName, const unsigned int newWindowSize[2],
                     WINDOW_MODE newWindowMode);
  ~MInstance();

  /*=============================================================================
    State control
    =============================================================================*/
  void start();
  void stop();

private:

  /*=============================================================================
    Debuging
    =============================================================================*/
  MDebugger *pDebugger{nullptr};
  //Timer
  std::chrono::time_point<std::chrono::steady_clock> performanceTestTimestamp{};
  bool performanceTestTimerStarted{false};
  void startPerformanceTestTimer();
  void stopPerformanceTestTimer() const;


  /*=============================================================================
    State control
    =============================================================================*/
  bool isRunning{false};

  /*=============================================================================
    Lifecycle
    =============================================================================*/
  void destroy();
  void startMainCallbackLoop();

  /*=============================================================================
    Instance info
    =============================================================================*/
  std::string appName;
  unsigned int windowSize[2]{0, 0};
  WINDOW_MODE windowMode;

  /*=============================================================================
    Subsystems
    =============================================================================*/
  // Vulkan
  MVulkanContext *pVulkanContext{nullptr};
  MRenderer *pRenderer{nullptr};

  // Window creation
  MWindowHandler *pWindowHandler{nullptr};
  std::unique_ptr<MWindowHandler> pInitialWindowHandler{nullptr};
};
} // namespace Magma