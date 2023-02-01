#include <map>
#include "muninregistration.h"

REGISTER_PLUGIN(servers) {
    if (ctx.isConfig()) {
        ctx.write("graph_title Total servers");
        ctx.write("graph_category servers");
        ctx.write("servers.label Servers");
        ctx.write("servers.min 0");
    }

    if (ctx.isFetch()) {
        int num = 0;

        ctx.lockServerData();
        for (const auto &pair : ctx.getServerData()) {
            if (pair.second.success()) {
                num++;
            }
        }
        ctx.unlockServerData();

        ctx.writef("servers.value %d", num);
    }
}

static std::map<uint32_t, std::string> RESPONSE_MAP = {
    { 5660023, "Success" },
    { 5660024, "Too fast" },
    { 5660025, "Banned" },
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
        ctx.write("graph_category servers");

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