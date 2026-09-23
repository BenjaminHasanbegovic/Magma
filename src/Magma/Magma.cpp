#include "../../include/Magma/Magma.h"

using namespace Magma;

    MInstance::MInstance(std::string  title):
title(std::move(title)){
      MDebugger::print("Magma instance created");
    }

    MInstance::~MInstance() {
          MDebugger::print("Destroying magma instance");
        delete debugger;
          debugger = nullptr;
    };

void MInstance::start() {
  MDebugger::print("Starting magma instance . . .");

};
