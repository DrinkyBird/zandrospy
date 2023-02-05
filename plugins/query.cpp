#include <map>
#include "muninregistration.h"
#include "muninnode.h"
#include "zanproto.h"

static std::map<uint32_t, std::string> RESPONSE_MAP = {
    { SERVER_LAUNCHER_CHALLENGE, "Success" },
    { SERVER_LAUNCHER_IGNORING, "Too fast" },
    { SERVER_LAUNCHER_BANNED, "Banned" },
    { -1, "Timed out" }
};

REGISTER_PLUGIN(servers_response) {
    std::unordered_map<uint32_t, int> map;
    for (const auto &pair : RESPONSE_MAP) {
        map[pair.first] = 0;
    }

    ctx.lockServerData();
    for (const auto &pair : ctx.getServerData()) {
        const auto &server = pair.second;
        if (map.count(server.response) == 0) {
            map[server.response] = 1;
        } else {
            map[server.response]++;
        }
    }
    ctx.unlockServerData();

    if (ctx.isConfig()) {
        ctx.write("graph_title Server responses");
        ctx.write("graph_category query");
        ctx.write("graph_vlabel Servers");

        for (const auto &pair : map) {
            if (RESPONSE_MAP.count(pair.first)) {
                ctx.writef("%d.label %s", pair.first, RESPONSE_MAP[pair.first].c_str());
            } else {
                ctx.writef("%d.label %d", pair.first, pair.first);
            }
            ctx.writef("%d.min 0", pair.first);
            ctx.writef("%d.draw AREASTACK", pair.first);
        }
    }

    if (ctx.isFetch()) {
        for (const auto &pair : map) {
            ctx.writef("%d.value %d", pair.first, pair.second);
        }
    }
}

REGISTER_PLUGIN(query_time) {
    if (ctx.isConfig()) {
        ctx.write("graph_title Query time");
        ctx.write("graph_args --base 1000 -l 0");
        ctx.write("graph_category query");
        ctx.write("graph_vlabel Duration (seconds)");
        ctx.write("time.label Query duration");
        ctx.write("time.info Time taken to query the master server and all game servers");
    }

    if (ctx.isFetch()) {
        ctx.lockQueryStats();
        const double time = ctx.getQueryStats().queryTime;
        ctx.unlockQueryStats();

        ctx.writef("time.value %.6f", time);
    }
}

REGISTER_PLUGIN(query_traffic) {
    if (ctx.isConfig()) {
        ctx.write("graph_title Total query traffic");
        ctx.write("graph_order msin msout sqin sqout");
        ctx.write("graph_args --base 1000");
        ctx.write("graph_vlabel bytes in (-) / out (+) per refresh");
        ctx.write("graph_category query");
        ctx.write("sqin.label received");
        ctx.write("sqin.type DERIVE");
        ctx.write("sqin.graph no");
        ctx.write("sqin.min 0");
        ctx.write("sqout.label Game servers");
        ctx.write("sqout.type DERIVE");
        ctx.write("sqout.negative sqin");
        ctx.write("sqout.min 0");
        ctx.write("msin.label received");
        ctx.write("msin.type DERIVE");
        ctx.write("msin.graph no");
        ctx.write("msin.min 0");
        ctx.write("msout.label Master server");
        ctx.write("msout.type DERIVE");
        ctx.write("msout.negative msin");
        ctx.write("msout.min 0");
    }

    if (ctx.isFetch()) {
        ctx.lockQueryStats();
        const auto &stats = ctx.getQueryStats();
        const uint32_t sqin = stats.queryTrafficIn;
        const uint32_t sqout = stats.queryTrafficOut;
        const uint32_t msin = stats.masterTrafficIn;
        const uint32_t msout = stats.masterTrafficOut;
        ctx.unlockQueryStats();

        ctx.writef("sqin.value %u", sqin);
        ctx.writef("sqout.value %u", sqout);
        ctx.writef("msin.value %u", msin);
        ctx.writef("msout.value %u", msout);
    }
}

REGISTER_PLUGIN(munin_time) {
    if (ctx.isConfig()) {
        ctx.write("graph_title Munin connection time");
        ctx.write("graph_args --base 1000 -l 0");
        ctx.write("graph_category query");
        ctx.write("graph_vlabel Duration (seconds)");
        ctx.write("time.label Connection duration");
    }

    if (ctx.isFetch()) {
        ctx.writef("time.value %.6f", ctx.getNode()->getLastConnectionTime());
    }
}
