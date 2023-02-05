#pragma once
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <ratio>
#include <type_traits>

using MeasureClock = std::conditional_t<
    std::chrono::high_resolution_clock::is_steady,
    std::chrono::high_resolution_clock,
    std::chrono::steady_clock>;

static_assert(MeasureClock::is_steady, "Clock is not monotonic.");
static_assert(std::ratio_less_equal<MeasureClock::period, std::micro>::value, "Clock is not precise enough (must be microsecond or better)");

template <typename Out>
static inline void split(const std::string &s, char delim, Out result) {
    std::istringstream iss(s);
    std::string item;
    while (std::getline(iss, item, delim)) {
        *result++ = item;
    }
}

static inline std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    split(s, delim, std::back_inserter(elems));
    return elems;
}

static std::string filterKey(const std::string &ver) {
    std::stringstream ss;
    for (const char &c : ver) {
        if (!std::isalnum(c)) {
            ss << '_';
        } else {
            ss << static_cast<char>(std::tolower(c));
        }
    }
    return ss.str();
}

static std::string lowercase(const std::string &other) {
    std::stringstream ss;
    for (const char &c : other) {
        ss << static_cast<char>(std::tolower(c));
    }
    return ss.str();
}

template<typename T>
inline constexpr double toSeconds(typename T::duration duration) {
    return static_cast<double>(std::chrono::duration_cast<std::chrono::microseconds>(duration).count()) / 1000000.0;
}