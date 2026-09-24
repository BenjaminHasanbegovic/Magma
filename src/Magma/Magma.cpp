#include "../../include/Magma/Magma.h"

using namespace Magma;

    MInstance::MInstance(std::string  appName):
appName(std::move(appName)){
      MDebugger::print("Magma instance created.");
    }

    MInstance::~MInstance() {
        MDebugger::print("Destroying magma instance.");
        //The order of destroying objects matters!!!
        delete vulkanContext;
        vulkanContext = nullptr;

        delete pDebugger;
          pDebugger = nullptr;
    };

void MInstance::start() {
  MDebugger::print("Starting magma instance . . .");
    vulkanContext = new MVulkanContext(appName, *pDebugger);
    MDebugger::print("Created magma instance.");
};
