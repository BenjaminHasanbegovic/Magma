#include "../../include/Magma/Magma.h"

using namespace Magma;

    /* # =============================================================================
       #  Class init and destruction
       # ============================================================================= */
MInstance::MInstance(std::string appName, const unsigned int newWindowSize[2], WINDOW_MODE newWindowMode)
    : appName(std::move(appName)), windowMode(newWindowMode) {
    //Class value init
    windowSize[0] = newWindowSize[0];
    windowSize[1] = newWindowSize[1];

    //Subsystems
    pDebugger = new MDebugger(this->appName);
    pVulkanContext = new MVulkanContext(appName, *pDebugger);
    pInitialWindowHandler = MWindowHandler::create(appName, windowMode, windowSize, *pDebugger);
    pSwapchain = new MSwapchain(*pDebugger);

    pDebugger->print("Initializing magma instance.");
}

MInstance::~MInstance() {
    destroy();
};

/* # =============================================================================
   #   State control
   # ============================================================================= */
void MInstance::start() {
    isRunning = true;
    pDebugger->print("Starting magma instance . . .");

    //Subsystems
    //Vulkan init
    try {
        MVulkanContext::setOSExtensions(MWindowHandler::getUserOS(), MWindowHandler::getOSSpecificExtensions());
        pVulkanContext->createInstance();

        pWindowHandler = pInitialWindowHandler.release();
        pWindowHandler->initWindow();
        pWindowHandler->createWindow();

        pWindowHandler->createVulkanSurface(pVulkanContext->getVkInstance());
        pVulkanContext->setVkSurface(pWindowHandler->getVkSurface());

        pVulkanContext->selectPhysicalDevice();
        pVulkanContext->findMandatoryQueueFamilies();
        pVulkanContext->createLogicalDevice();

        pSwapchain->querySupport(pVulkanContext->getVkPhysicalDeviceDevice(),pWindowHandler->getVkSurface());

        isRunning = true;
        pDebugger->print("Created magma instance.");
        startMainCallbackLoop();
    }catch (std::exception& e) {
        pDebugger->print(std::string("Initialization aborted:\n ") + e.what(), MESSAGE_TYPE::ERROR);
        destroy();
    };
};

void MInstance::stop() {
    if (isRunning) {
        isRunning = false;
        pDebugger->print("Stopped magma instance.");
    } else {
        pDebugger->print("The magma instance is not running, cannot stop.", MESSAGE_TYPE::WARNING);
    };
};


/* # =============================================================================
   #  Instance lifecycle
   # ============================================================================= */
void MInstance::startMainCallbackLoop() {
    pDebugger->print("Starting main loop . . .");
    while (isRunning) {
        pWindowHandler->pollEvents(isRunning);
    };
    pDebugger->print("Main loop ended.");
};

void MInstance::destroy() {
    if (isDestroyed) {
        return;
    }

    if (pDebugger) {
        pDebugger->print("Destroying magma instance . . .");
    }
    //The order of destruction is important
    delete pSwapchain;
    pSwapchain = nullptr;

    pInitialWindowHandler.reset();
    pInitialWindowHandler = nullptr;

    delete pWindowHandler;
    pWindowHandler = nullptr;

    delete pVulkanContext;
    pVulkanContext = nullptr;

    delete pDebugger;
    pDebugger = nullptr;

    isRunning = false;
    isDestroyed = true;
}
