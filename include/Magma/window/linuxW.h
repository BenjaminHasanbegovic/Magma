#pragma once

#include "../Magma.h"
#include "windowHandler.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

namespace Magma {
    class MLinuxHandler : public MWindowHandler {
    public:
        MLinuxHandler(const std::string& appName, WINDOW_MODE windowMode, const unsigned int windowSize[2], MDebugger& debugger);
        ~MLinuxHandler() override;

        void initWindow() override;
        void pollEvents() override;

        [[nodiscard]] Display* getX11Display() const { return display; }
        [[nodiscard]] Window getX11Window() const { return window; }
        [[nodiscard]] Atom getWmDeleteMessage() const { return wmDeleteMessage; }

    private:
        Display* display{nullptr};
        unsigned long int screen{0};
        unsigned long backgroundColor;
        Window window;
       Atom wmDeleteMessage{0};
    };
}