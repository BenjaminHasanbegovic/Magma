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
      MWindowHandler::create(this->appName, windowMode, windowSize, *pDebugger);

  pDebugger->print("Constructed magma instance.");
};

MInstance::~MInstance() { destroy(); };

/*=============================================================================
  State control
  =============================================================================*/
void MInstance::start() {
  pDebugger->print("Starting magma instance . . .");
  startPerformanceTestTimer();
  try {
    MVulkanContext::setOSExtensions(MWindowHandler::getUserOS(),
                                    MWindowHandler::getOSSpecificExtensions());
    pVulkanContext->createInstance();

    pWindowHandler = pInitialWindowHandler.release(); // Set the window handler to OS specific
    pWindowHandler->initWindow();
    pWindowHandler->createWindow();
    pWindowHandler->changeCursorStyle(CURSOR_STYLE::LOADING);

    pWindowHandler->createSurface(pVulkanContext->getInstance());
    pVulkanContext->setSurface(pWindowHandler->getSurface());

    pVulkanContext->selectPhysicalDevice();
    pVulkanContext->findMandatoryQueueFamilyIndexes();
    pVulkanContext->createLogicalDevice();

    pRenderer = new MRenderer(*pDebugger, pVulkanContext->getPhysicalDevice(),
      pVulkanContext->getLogicalDevice(),pWindowHandler->getSurface(),
      pVulkanContext->getGraphicsQueue(),pVulkanContext->getPresentQueue());

    pRenderer->querySwapchainSupport();
    pRenderer->createSwapchain();
    pRenderer->createImageViews();

    pRenderer->createRenderPass();
    pRenderer->createFramebuffers();

    pRenderer->createCommandPool(MVulkanContext::getGraphicsQueueFamilyIndex());

    pRenderer->createCommandBuffers();
    pRenderer->createGraphicsPipeline();
    pRenderer->createSemaphores();

    pRenderer->recordCommandBuffer();

    isRunning = true;
    pWindowHandler->changeCursorStyle(CURSOR_STYLE::POINTER);
    pDebugger->print("Created magma instance.");
    stopPerformanceTestTimer();
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
  pDebugger->print("Destroying magma instance . . .");
  startPerformanceTestTimer();
  // The order of destruction is important !!!
  // Render handle
  delete pRenderer;
  pRenderer = nullptr;

  // Window handle
  pInitialWindowHandler.reset();
  pInitialWindowHandler = nullptr;
  delete pWindowHandler;
  pWindowHandler = nullptr;

  // Vulkan context handle
  delete pVulkanContext;
  pVulkanContext = nullptr;

  stopPerformanceTestTimer();
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
      pRenderer->setExtent(width, height);
      pRenderer->recordCommandBuffer();
      pWindowHandler->framebufferResized = false;
    };
    pRenderer->drawFrame();
  };
  pDebugger->print("Main loop ended.");
};

/*=============================================================================
  Debugging
  =============================================================================*/
void MInstance::startPerformanceTestTimer() {
  if (!DEBUG_MODE  && !performanceTestTimerStarted) return;
  performanceTestTimestamp = std::chrono::steady_clock::now();
  pDebugger->print("Started performance timer.");
}

void MInstance::stopPerformanceTestTimer() {
  if (!DEBUG_MODE) return;
  auto endTime = std::chrono::steady_clock::now();

  // Calculate elapsed time in milliseconds
  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
      endTime - performanceTestTimestamp
  ).count();

  pDebugger->print("Stopped performance test timer.\nFinal time: " + std::to_string(duration) + " ms.");
}