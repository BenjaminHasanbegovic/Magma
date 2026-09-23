#pragma once

#include "Debugger.h"
class MDebugger;

#include <utility>
#include <string>

namespace Magma {

    class MInstance {
    public:
        explicit  MInstance(std::string  title = "Magma Application");
        ~MInstance();

        void start();
    private:
        //Debugging
        MDebugger* debugger =  new MDebugger();

        //Instance info
        std::string title;

        //Data share



    };
};