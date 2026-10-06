#pragma once

#include "Uitls/Debugger.h"
#include "window/windowHandler.h"
#include "Uitls/ErrorCodes.h"
#include "vulkan/VulkanContext.h"
#include "vulkan/Swapchain.h"

#include <string>

namespace Magma {

    /* # =============================================================================
       #  External classes
       # ============================================================================= */
    class MVulkanContext;
    class MWindowHandler;
    class MDebugger;
    class MSwapchain;

 /* # =============================================================================
    #  Main class
    # ============================================================================= */
    class MInstance {
    public:

 /* # =============================================================================
    #  Class init and destruction
    # ============================================================================= */
    explicit MInstance(std::string appName, const unsigned int newWindowSize[2], WINDOW_MODE newWindowMode);
    ~MInstance();

 /* # =============================================================================
         #   State control
         # ============================================================================= */
        void start();
        void stop();

    private:
        /* # =============================================================================
           #  Debugging
           # ============================================================================= */
        MDebugger* pDebugger{nullptr};

        /* # =============================================================================
           #  State control
           # ============================================================================= */
        bool isRunning{false};

       /* # =============================================================================
          # Lifecycle
          # ============================================================================= */
        void destroy();
        void startMainCallbackLoop();

        /* # =============================================================================
           #  Instance info
           # ============================================================================= */
        std::string appName;
        unsigned int windowSize[2]{0, 0};
        WINDOW_MODE windowMode;

        /* # =============================================================================
           #  Subsystems
           # ============================================================================= */
       //Vulkan
        MVulkanContext* pVulkanContext{nullptr};
        MSwapchain* pSwapchain{nullptr};

       //Window creation
        MWindowHandler* pWindowHandler{nullptr};
        std::unique_ptr<MWindowHandler> pInitialWindowHandler{nullptr};
    };
}