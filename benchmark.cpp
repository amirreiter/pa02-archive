#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <sstream>
#include <string>
#include <vector>

#include <mach/mach_time.h>
#include <sys/resource.h>

// most of this stuff is from
// https://developer.apple.com/documentation/kernel/1462446-mach_absolute_time

namespace bench {

inline uint64_t wall_ns() {
    static mach_timebase_info_data_t tb = [] {
        mach_timebase_info_data_t t{};
        mach_timebase_info(&t);
        return t;
    }();
    return mach_absolute_time() * tb.numer / tb.denom;
}

inline uint64_t cpu_ns() {
    struct rusage ru{};
    getrusage(RUSAGE_SELF, &ru);
    return static_cast<uint64_t>(ru.ru_utime.tv_sec  + ru.ru_stime.tv_sec)  * 1000000000ULL
         + static_cast<uint64_t>(ru.ru_utime.tv_usec + ru.ru_stime.tv_usec) * 1000ULL;
}

inline std::string fmt_ns(double ns) {
    std::ostringstream o;
    if      (ns >= 1e9) o << std::fixed << std::setprecision(3) << ns/1e9 << " s ";
    else if (ns >= 1e6) o << std::fixed << std::setprecision(3) << ns/1e6 << " ms";
    else if (ns >= 1e3) o << std::fixed << std::setprecision(3) << ns/1e3 << " us";
    else                o << std::fixed << std::setprecision(3) << ns      << " ns";
    return o.str();
}

class Run {
public:
    void start() { w0_ = wall_ns(); c0_ = cpu_ns(); }
    void end()   { wall_.push_back(wall_ns() - w0_); cpu_.push_back(cpu_ns() - c0_); }

    void analysis() const {
        auto compute = [](std::vector<uint64_t> s) {
            std::sort(s.begin(), s.end());
            double sum = std::accumulate(s.begin(), s.end(), 0.0);
            double mean = sum / s.size();
            double med = s.size() % 2 == 0
                ? (s[s.size()/2 - 1] + s[s.size()/2]) / 2.0
                : s[s.size()/2];
            double sq = 0;
            for (auto v : s) sq += (v - mean) * (v - mean);
            double stddev = std::sqrt(sq / s.size());
            struct R { double min, max, mean, median, stddev; };
            return R{ (double)s.front(), (double)s.back(), mean, med, stddev };
        };

        auto w = compute(wall_);
        auto c = compute(cpu_);

        auto row = [&](const char* label, auto& r) {
            std::cout << "  " << std::left << std::setw(6) << label
                      << "  min="    << std::setw(12) << fmt_ns(r.min)
                      << "  max="    << std::setw(12) << fmt_ns(r.max)
                      << "  mean="   << std::setw(12) << fmt_ns(r.mean)
                      << "  median=" << std::setw(12) << fmt_ns(r.median)
                      << "  stddev=" << fmt_ns(r.stddev) << '\n';
        };

        std::cout << wall_.size() << " reps\n";
        row("wall", w);
        row("cpu",  c);
    }

private:
    std::vector<uint64_t> wall_, cpu_;
    uint64_t w0_ = 0, c0_ = 0;
};

} // namespace bench
