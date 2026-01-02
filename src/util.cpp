#include "util.h"

#include <chrono>
#include <thread>
#include <iostream>

namespace util {
    void timed_print(const std::string_view msg, int ms) {
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
        std::cout << msg << std::endl;
    }
}