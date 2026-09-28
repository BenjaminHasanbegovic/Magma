#pragma once

#include <string>
#include <thread>
#include <ctime>
#include <chrono>

namespace Magma {
           /* # =============================================================================
              #  Essential class values
              # ============================================================================= */
#ifndef DEBUG_MODE
#define DEBUG_MODE 0
#endif

    enum class MESSAGE_TYPE:int8_t {
        INFO = 0,
        WARNING = 1,
        ERROR = 2,
    };

    /* # =============================================================================
       #  Main class
       # ============================================================================= */
    class MDebugger { //Keep the class and it's methods simple
    public:
       /* # =============================================================================
          #  Class init and destruction
          # ============================================================================= */
        explicit MDebugger(const std::string& id);
       ~MDebugger();

        /* # =============================================================================
           #  Printing logic
           # ============================================================================= */
         void print(const std::string& message , MESSAGE_TYPE type = MESSAGE_TYPE::INFO) const;

private:
       /* # =============================================================================
          #  Debug logic
          # ============================================================================= */
        static constexpr bool debugMode {DEBUG_MODE};

        /* # =============================================================================
           #  Message info
           # ============================================================================= */
        std::string id;
         static std::pair<std::thread::id, std::string> getMessageInfo() {
           //Message info
          const std::thread::id messageThread = std::this_thread::get_id();  //Get the thread from with the print was called
          const std::time_t messageTimestamp = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()); //Get the day,month,day,time and the year in which the message was printed

            //Formating
          std::string timeStr = std::ctime(&messageTimestamp);
          if (!timeStr.empty() && timeStr.back() == '\n') {
             timeStr.pop_back();
          }

          return {messageThread, timeStr};
       }
};
};
