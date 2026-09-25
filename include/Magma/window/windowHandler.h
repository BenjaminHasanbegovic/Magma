#pragma once

#include "../Debugger.h"
#include "../ErrorCodes.h"

#include <string>
#include <memory> // For std::unique_ptr

namespace Magma {
    enum WINDOW_MODE : unsigned int {
        BORDERLESS = 0,
        FULL_SCREEN = 1,
        BORDERLESS_FULL_SCREEN = 2,
        OVERLAY = 3,
    };

    class MWindowHandler {
    public:
        static std::unique_ptr<MWindowHandler> create(const std::string& appName, WINDOW_MODE windowMode, const unsigned int windowSize[2], MDebugger& debugger);

        MWindowHandler(const std::string& appName, WINDOW_MODE windowMode, const unsigned int windowSize[2], MDebugger& debugger);
        virtual ~MWindowHandler();

        virtual void initWindow() = 0;
        virtual void pollEvents() = 0;
    protected:
        std::string appName;
        WINDOW_MODE mode;
        unsigned int size[2];
        static ErrorCode errorCode;
        MDebugger* pDebugger{nullptr};
    };
}