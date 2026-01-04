#pragma once

#include <string_view>
#include <chrono>

using time_point = std::chrono::time_point<std::chrono::high_resolution_clock>;
namespace util {
    void timed_print(const std::string_view msg, int ms);
    time_point get_time_point_now();
}