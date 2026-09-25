#include "../../include/Magma/Debugger.h"

#include <iostream>

using namespace Magma;

    MDebugger::MDebugger(const std::string& id):id(id){
        print("Debugger started.");
    };

    MDebugger::~MDebugger() {
        print("Debugger destroyed.");
    };

   void MDebugger::print(const std::string& message , const MESSAGE_TYPE type) {
       if (debugMode && !message.empty()) {
       std::string prefix;
       std::string messageColor;
       const std::string resetMessageColor = "\033[0m";

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

       const std::thread::id messageThread = std::this_thread::get_id();
           const auto messageTimestamp = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
           std::string timeStr = std::ctime(&messageTimestamp);
           timeStr.pop_back();

           constexpr auto COLOR_RESET  = "\033[0m";
           constexpr auto COLOR_GRAY   = "\033[90m";
           constexpr auto COLOR_CYAN   = "\033[36m";
           constexpr auto COLOR_BOLD   = "\033[1m";

           std::cout << COLOR_GRAY << timeStr << COLOR_RESET << " : ["
                     << messageColor << COLOR_BOLD << prefix << COLOR_RESET << "] "
                     << COLOR_GRAY << "(id:" << COLOR_CYAN << id
                     << COLOR_GRAY << ", Thread:" << COLOR_CYAN << messageThread
                     << COLOR_GRAY << ") " << COLOR_RESET
                     << message << std::endl;
       };
   };

