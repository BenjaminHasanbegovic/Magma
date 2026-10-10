#include "../../../include/Magma/Utils/Debugger.h"

#include <iostream>

using namespace Magma;

     /* # =============================================================================
        #  Class init and destruction
        # ============================================================================= */
    MDebugger::MDebugger(const std::string& id):id(id){
        print("Debugger started.");
    };

    MDebugger::~MDebugger() {
        print("Debugger destroyed.");
    };

    /* # =============================================================================
       #  Printing logic
       # ============================================================================= */
   void MDebugger::print(const std::string& message , const MESSAGE_TYPE type) const {
       if (debugMode && !message.empty()) {
           //Values that define the output context & style
       std::string prefix;
       std::string messageColor;
       const std::string resetMessageColor = "\033[0m";
           auto [messageThread, timeStr] = getMessageInfo();  //Extract info

           //Styling colors
           constexpr auto COLOR_RESET  = "\033[0m";
           constexpr auto COLOR_GRAY   = "\033[90m";
           constexpr auto COLOR_CYAN   = "\033[36m";
           constexpr auto COLOR_BOLD   = "\033[1m";

           //Styling
       switch (type) {
           case MESSAGE_TYPE::INFO:
               prefix = "INFO";
               messageColor = resetMessageColor;
               break;
           case MESSAGE_TYPE::WARNING:
               prefix = "WARNING";
               messageColor = "\033[33m";
               break;
           case MESSAGE_TYPE::ERROR:
               prefix = "ERROR";
               messageColor = "\033[31m";
               break;
       };

           //Printing the message
           std::cout << COLOR_GRAY << timeStr << COLOR_RESET << " : ["
                     << messageColor << COLOR_BOLD << prefix << COLOR_RESET << "] "
                     << COLOR_GRAY << "(id:" << COLOR_CYAN << id
                     << COLOR_GRAY << ", Thread:" << COLOR_CYAN << messageThread
                     << COLOR_GRAY << ") " << COLOR_RESET
                     << message << std::endl;
       };
   };

