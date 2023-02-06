#include <map>
#include "muninregistration.h"
#include "muninnode.h"
#include "zanproto.h"

static std::map<uint32_t, std::pair<std::string, std::string>> RESPONSE_MAP = {
    { SERVER_LAUNCHER_CHALLENGE, { "Success", "The query request succeeded" } },
    { SERVER_LAUNCHER_IGNORING, { "Too fast", "The request was ignored due to rate limiting" } },
    { SERVER_LAUNCHER_BANNED, { "Banned", "The query host is banned from this server" } },
    { -1, { "Timed out", "The request was not responded to in time or another error occurred" } }
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
                ctx.writef("%d.label %s", pair.first, RESPONSE_MAP[pair.first].first.c_str());
                ctx.writef("%d.info %s", pair.first, RESPONSE_MAP[pair.first].second.c_str());
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

REGISTER_PLUGIN(master_traffic) {
    if (ctx.isConfig()) {
        ctx.write("graph_title Total master server query traffic");
        ctx.write("graph_order in out");
        ctx.write("graph_args --base 1000");
        ctx.write("graph_vlabel bytes in (-) / out (+) per refresh");
        ctx.write("graph_category query");
        ctx.write("in.label received");
        ctx.write("in.graph no");
        ctx.write("in.min 0");
        ctx.write("out.label bytes");
        ctx.write("out.negative in");
        ctx.write("out.min 0");
    }

    if (ctx.isFetch()) {
        ctx.lockQueryStats();
        const auto &stats = ctx.getQueryStats();
        const uint32_t in = stats.masterTrafficIn;
        const uint32_t out = stats.masterTrafficOut;
        ctx.unlockQueryStats();

        ctx.writef("in.value %u", in);
        ctx.writef("out.value %u", out);
    }
}

REGISTER_PLUGIN(query_traffic) {
    if (ctx.isConfig()) {
        ctx.write("graph_title Total game server query traffic");
        ctx.write("graph_order in out");
        ctx.write("graph_args --base 1000");
        ctx.write("graph_vlabel bytes in (-) / out (+) per refresh");
        ctx.write("graph_category query");
        ctx.write("in.label received");
        ctx.write("in.graph no");
        ctx.write("in.min 0");
        ctx.write("out.label bytes");
        ctx.write("out.negative in");
        ctx.write("out.min 0");
    }

    if (ctx.isFetch()) {
        ctx.lockQueryStats();
        const auto &stats = ctx.getQueryStats();
        const uint32_t in = stats.queryTrafficIn;
        const uint32_t out = stats.queryTrafficOut;
        ctx.unlockQueryStats();

        ctx.writef("in.value %u", in);
        ctx.writef("out.value %u", out);
    }
}

REGISTER_PLUGIN(munin_time) {
    if (ctx.isConfig()) {
        ctx.write("graph_title Munin connection time");
        ctx.write("graph_args --base 1000 -l 0");
        ctx.write("graph_category query");
        ctx.write("graph_vlabel Duration (seconds)");
        ctx.write("time.label Connection duration");
        ctx.write("time.info Time taken during the last connection from the Munin host. This is always one update behind, and will be 0 after the node restarts.");
    }

    if (ctx.isFetch()) {
        ctx.writef("time.value %.6f", ctx.getNode()->getLastConnectionTime());
    }
}
