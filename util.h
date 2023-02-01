#pragma once
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>

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