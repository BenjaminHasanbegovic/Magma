#pragma once
#include <string>
#include <thread>
#include <ctime>
#include <chrono>


#ifndef DEBUG_MODE
#define DEBUG_MODE 0
#endif

namespace Magma {
    enum class MESSAGE_TYPE:int8_t {
        INFO = 0,
        WARNING = 1,
        ERROR = 2,
    };

    class MDebugger {
    public:
        explicit MDebugger();
    ~MDebugger();
        //Message logic
        static void print(const std::string& message , MESSAGE_TYPE type = MESSAGE_TYPE::INFO);
private:
        static constexpr bool debugMode {DEBUG_MODE};
};
};
