#pragma once

#include <cstdint>
#include <fstream>
#include <string>

#if defined(__APPLE__)
#include <mach/mach.h>
#endif

namespace mse {

// Best-effort process resident set size (0 if unsupported / unavailable).
[[nodiscard]] inline std::uint64_t rss_bytes() {
#if defined(__APPLE__)
    mach_task_basic_info info{};
    mach_msg_type_number_t count = MACH_TASK_BASIC_INFO_COUNT;
    if (task_info(mach_task_self(), MACH_TASK_BASIC_INFO, reinterpret_cast<task_info_t>(&info),
                  &count) != KERN_SUCCESS)
        return 0;
    return static_cast<std::uint64_t>(info.resident_size);
#elif defined(__linux__)
    std::ifstream f("/proc/self/status");
    std::string key;
    while (f >> key) {
        if (key == "VmRSS:") {
            std::uint64_t kb = 0;
            f >> kb;
            return kb * 1024;
        }
        std::string rest;
        std::getline(f, rest);
    }
    return 0;
#else
    return 0;
#endif
}

} // namespace mse
