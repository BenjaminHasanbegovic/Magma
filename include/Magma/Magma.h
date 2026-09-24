#pragma once

#include "Debugger.h"
class MDebugger;

#include "vulkan/VulkanContext.h"
class MVulkanContext;

#include <utility>
#include <string>

namespace Magma {

    class MInstance {
    public:
        explicit  MInstance(std::string  appName = "Magma Application");
        ~MInstance();

        void start();

    private:
        //Debugging
        MDebugger* pDebugger{new MDebugger()};

        //Instance info
        std::string appName;

        //Vulkan logic
        MVulkanContext* vulkanContext{nullptr};


    };
};