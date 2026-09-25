#include "../../include/Magma/Magma.h"
#include "../../include/Magma/vulkan/VulkanContext.h"
#include <iostream>

using namespace Magma;

MInstance::MInstance(std::string appName)
    : appName(std::move(appName)) {
    s_Instance = this;
    MDebugger::print("Initializing magma instance.");
}

MInstance::~MInstance() {
    destroy();
    if (s_Instance == this) {
        s_Instance = nullptr;
    }
}

void MInstance::requestDestruction(ErrorCode errorCode) {
    if (s_Instance) {
        s_Instance->tryDestruction(errorCode);
    }
}

void MInstance::tryDestruction(const ErrorCode errorCode) {
    MDebugger::print("Destruction requested for magma instance.\nREQUEST REASON: " +
                     std::string(errorCodeToString(errorCode)) +
                     " (CODE: " + std::to_string(static_cast<uint32_t>(errorCode)) + ")",
                     MESSAGE_TYPE::ERROR);

    if (isRunning) {
        stop();
    }
    destroy();
}

void MInstance::start() {
    isRunning = true;
    MDebugger::print("Starting magma instance . . .");

    vulkanContext = new MVulkanContext(appName, *pDebugger);

    if (isRunning) {
        MDebugger::print("Created magma instance.");
    }else {
        MDebugger::print("Failed to create magma instance.", MESSAGE_TYPE::ERROR);
    }
};

void MInstance::stop() {
    if (isRunning) {
        isRunning = false;
        MDebugger::print("Stopped magma instance.");
    } else {
        MDebugger::print("The magma instance is not running; cannot stop.", MESSAGE_TYPE::WARNING);
    }
}

void MInstance::destroy() {
    MDebugger::print("Destroying magma instance . . .");

    delete vulkanContext;
    vulkanContext = nullptr;

    delete pDebugger;
    pDebugger = nullptr;

    isRunning = false;
}