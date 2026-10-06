#include "../../include/Magma/Magma.h"

using namespace Magma;

/*=============================================================================
  Class init and destruction
  =============================================================================*/
MInstance::MInstance(std::string appName, const unsigned int newWindowSize[2],
                     const WINDOW_MODE newWindowMode)
    : appName(std::move(appName)), windowMode(newWindowMode) {
  windowSize[0] = newWindowSize[0]; // Set window width
  windowSize[1] = newWindowSize[1]; // Set window height

  pDebugger = new MDebugger(this->appName);
  pVulkanContext = new MVulkanContext(appName, *pDebugger);
  pInitialWindowHandler =
      MWindowHandler::create(appName, windowMode, windowSize, *pDebugger);

  pDebugger->print("Initializing magma instance.");
};

MInstance::~MInstance() { destroy(); };

/*=============================================================================
  State control
  =============================================================================*/
void MInstance::start() {
  pDebugger->print("Starting magma instance . . .");

  isRunning = true;

  try {
    MVulkanContext::setOSExtensions(MWindowHandler::getUserOS(),
                                    MWindowHandler::getOSSpecificExtensions());
    pVulkanContext->createInstance();

    pWindowHandler = pInitialWindowHandler
                         .release(); // Set the window handler to OS specific
    pWindowHandler->initWindow();
    pWindowHandler->createWindow();

    pWindowHandler->createVulkanSurface(pVulkanContext->getInstance());
    pVulkanContext->setSurface(pWindowHandler->getVkSurface());

    pVulkanContext->selectPhysicalDevice();
    pVulkanContext->findMandatoryQueueFamilies();
    pVulkanContext->createLogicalDevice();

    pSwapchain = new MSwapchain(
        *pDebugger, pVulkanContext->getPhysicalDeviceDevice(),
        pVulkanContext->getLogicalDevice(), pWindowHandler->getVkSurface());
    pSwapchain->querySupport();
    pSwapchain->create();

    isRunning = true;
    pDebugger->print("Created magma instance.");
    startMainCallbackLoop();
  } catch (std::exception &e) {
    isRunning = false;
    pDebugger->print(std::string("Initialization aborted:\n ") + e.what(),
                     MESSAGE_TYPE::ERROR);
    destroy();
  };
};

void MInstance::stop() {
  if (isRunning) {
    isRunning = false;
    pDebugger->print("Stopped magma instance.");
  } else {
    pDebugger->print("The magma instance is not running, cannot stop.",
                     MESSAGE_TYPE::WARNING);
  };
};

/*=============================================================================
  Lifecycle
  =============================================================================*/
void MInstance::destroy() {
  if (pDebugger) {
    pDebugger->print("Destroying magma instance . . .");
  };

  // The order of destruction is important !!!
  // Swapchain handle
  delete pSwapchain;
  pSwapchain = nullptr;

  // Window handle
  pInitialWindowHandler.reset();
  pInitialWindowHandler = nullptr;
  delete pWindowHandler;
  pWindowHandler = nullptr;

  // Vulkan context handle
  delete pVulkanContext;
  pVulkanContext = nullptr;

  // Debugger handle
  delete pDebugger;
  pDebugger = nullptr;

  isRunning = false;
};

void MInstance::startMainCallbackLoop() {
  pDebugger->print("Starting main loop . . .");
  while (isRunning) {
    pWindowHandler->pollEvents(isRunning);
    if (pWindowHandler->framebufferResized) {
      uint32_t width, height;
      pWindowHandler->getFramebufferSize(width, height);
      pSwapchain->setExtent(width, height);

      pWindowHandler->framebufferResized = false;
    };
  };
  pDebugger->print("Main loop ended.");
};