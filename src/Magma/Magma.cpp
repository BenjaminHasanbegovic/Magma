#include "../../include/Magma/Magma.h"
#include "../../include/Magma/vulkan/VulkanContext.h"

using namespace Magma;

    /* # =============================================================================
       #  Class init and destruction
       # ============================================================================= */
MInstance::MInstance(std::string appName, const unsigned int newWindowSize[2], WINDOW_MODE newWindowMode)
    : appName(std::move(appName)), windowMode(newWindowMode) {
    //Class value init
    windowSize[0] = newWindowSize[0];
    windowSize[1] = newWindowSize[1];
    s_Instance = this;
    //Subsystems
    pDebugger = new MDebugger(this->appName);
    pVulkanContext = new MVulkanContext(appName, *pDebugger);
    pInitialWindowHandler = MWindowHandler::create(appName, windowMode, windowSize, *pDebugger);

    pDebugger->print("Initializing magma instance.");
}

MInstance::~MInstance() {
    destroy();
    if (s_Instance == this) {
        s_Instance = nullptr;
    };
};

/* # =============================================================================
 #  Instance lifecycle
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

        pDebugger->print("Created magma instance.");
    }catch (std::exception& e) {
        pDebugger->print(std::string("Initialization aborted:\n ") + e.what(), MESSAGE_TYPE::ERROR);
    }
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
   #  State control
   # ============================================================================= */
void MInstance::destroy() {
    if (isDestroyed) {
        return;
    }

    if (pDebugger) {
        pDebugger->print("Destroying magma instance . . .");
    }
    //The order of destruction is important
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

void MInstance::requestDestruction(const ErrorCode errorCode) { //The function is here so external classes can invoke destruction
    if (s_Instance) {
        s_Instance->tryDestruction(errorCode);
    };
};

void MInstance::tryDestruction(const ErrorCode errorCode) {
    if (pDebugger) {
        pDebugger->print("Destruction requested for magma instance.\nREQUEST REASON: " +
                         std::string(errorCodeToString(errorCode)) +
                         " (CODE: " + std::to_string(static_cast<uint32_t>(errorCode)) + ")",
                         MESSAGE_TYPE::ERROR);
    };

    //Lifecycle end
    if (isRunning) {
        stop();
    };
    destroy();
};