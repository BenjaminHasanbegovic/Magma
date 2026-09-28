#pragma once

#include "../Uitls/Debugger.h"
#include "../Uitls/ErrorCodes.h"
#include <string>
#include <memory>

namespace Magma {
            /* # =============================================================================
               #  Essential class values
               # ============================================================================= */
    enum WINDOW_MODE : unsigned int {
        BORDERLESS = 0,
        FULL_SCREEN = 1,
        BORDERLESS_FULL_SCREEN = 2,
        OVERLAY = 3,
    };
            /* # =============================================================================
               #  Main class
               # ============================================================================= */
    class MWindowHandler { //This class will be parent to any new OS handler implementation, it is just here to be overridden
    public:
        static std::unique_ptr<MWindowHandler> create(const std::string& appName, WINDOW_MODE windowMode, const unsigned int windowSize[2], MDebugger& debugger);

        /* # =============================================================================
           #  Class init and destruction
           # ============================================================================= */
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
};