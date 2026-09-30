#pragma once

#include "Uitls/Debugger.h"
#include "./window/windowHandler.h"
#include "Uitls/ErrorCodes.h"
#include <string>

namespace Magma {
    /* # =============================================================================
       #  External classes
       # ============================================================================= */
    class MVulkanContext;
    class MWindowHandler;
    class MDebugger;

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
         #  Instance lifecycle
         # ============================================================================= */
        void start();
        void stop();

        /* # =============================================================================
          #  Error handling
          # ============================================================================= */
        void tryDestruction(ErrorCode errorCode = ErrorCode::NO_ERRORS);
        static void requestDestruction(ErrorCode errorCode = ErrorCode::NO_ERRORS);
        [[nodiscard]] static MInstance* getInstance() { return s_Instance; }

    private:
        /* # =============================================================================
           #  Debugging
           # ============================================================================= */
        MDebugger* pDebugger{nullptr};

        /* # =============================================================================
           #  State control
           # ============================================================================= */
        static inline MInstance* s_Instance{nullptr};
        void destroy();
        bool isRunning{false};
        bool isDestroyed{false};

        /* # =============================================================================
           #  Instance info
           # ============================================================================= */
        std::string appName;
        unsigned int windowSize[2]{0, 0};
        WINDOW_MODE windowMode;

        /* # =============================================================================
           #  Subsystems
           # ============================================================================= */
        MVulkanContext* pVulkanContext{nullptr};
        MWindowHandler* windowHandler{nullptr};
        std::unique_ptr<MWindowHandler> pWindowHandler{nullptr};
    };
}