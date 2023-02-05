#pragma once
#include <cstdint>

struct QueryStats {
    QueryStats() :
        queryTime(0.0), queryTrafficIn(0), queryTrafficOut(0) { }

    double queryTime;
    uint32_t queryTrafficIn, queryTrafficOut;
    uint32_t masterTrafficIn, masterTrafficOut;
};
