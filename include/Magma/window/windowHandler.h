#pragma once

#include <string>

namespace Magma {
    class windowHandler {
    public:
        windowHandler();
        ~windowHandler();
    private:
        static std::string getOSName();
    };
};