#pragma once

#include "Debugger.h"
#include "./window/windowHandler.h"
#include "ErrorCodes.h"

#include <string>

namespace Magma {
    class MVulkanContext;

    class MInstance {
    public:
        explicit MInstance(std::string appName, const unsigned int newWindowSize[2], WINDOW_MODE newWindowMode);
        ~MInstance();

        // Engine lifecycle
        void start();
        void stop();

        // Error handling
        void tryDestruction(ErrorCode errorCode = ErrorCode::NO_ERRORS);
        static void requestDestruction(ErrorCode errorCode = ErrorCode::NO_ERRORS);
        [[nodiscard]] static MInstance* getInstance() { return s_Instance; }

    private:
        static inline MInstance* s_Instance{nullptr};

        // Debugging
        MDebugger* pDebugger{nullptr};

        // State control
        void destroy();
        bool isDestroyed{false};

        // Instance info
        std::string appName;
        unsigned int windowSize[2]{0, 0};
        WINDOW_MODE windowMode;
        bool isRunning{false};

        // Subsystems
        MVulkanContext* vulkanContext{nullptr};
        MWindowHandler* windowHandler{nullptr};
    };
}