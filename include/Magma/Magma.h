#pragma once

#include "Debugger.h"
class MDebugger;

#include "ErrorCodes.h"
#include <utility>
#include <string>

namespace Magma {
    class MVulkanContext;

    class MInstance {
    public:
        explicit  MInstance(std::string  appName = "Magma Application");
        ~MInstance();

        //Magma logic
        void start();
        void stop();

        //Error handling
        void tryDestruction(ErrorCode errorCode = ErrorCode::NO_ERRORS);
        static void requestDestruction(ErrorCode errorCode = ErrorCode::NO_ERRORS);
        [[nodiscard]] static MInstance* getInstance() { return s_Instance;};

    private:
        //Debugging
        MDebugger* pDebugger{new MDebugger()};

        //Magma logic
        static inline MInstance* s_Instance{nullptr};
        void destroy();

        //Instance info
        std::string appName;
        bool isRunning{false};

        //Vulkan logic
        MVulkanContext* vulkanContext{nullptr};


    };
};