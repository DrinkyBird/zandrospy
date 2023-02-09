#include <map>
#include "muninregistration.h"

REGISTER_PLUGIN(servers) {
    if (ctx.isConfig()) {
        ctx.write("graph_title Total servers");
        ctx.write("graph_category servers");
        ctx.write("graph_vlabel Servers");
        ctx.write("servers.label Servers");
        ctx.write("servers.min 0");
        ctx.write("servers.info Total number of servers on the master that returned a response to the querent");
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

REGISTER_PLUGIN(servers_chain) {
    std::unordered_map<int, int> map;
    for (const auto &pair : SERVER_CHAIN_MAP) {
        map[pair.first] = 0;
    }

    ctx.lockServerData();
    for (const auto &pair : ctx.getServerData()) {
        const auto &server = pair.second;
        map[server.serverChain]++;
    }
    ctx.unlockServerData();

    if (ctx.isConfig()) {
        ctx.write("graph_title Servers by server chain");
        ctx.write("graph_category servers");
        ctx.write("graph_vlabel Servers");

        for (const auto &pair : map) {
            ctx.writef("c%d.label %s", pair.first, SERVER_CHAIN_MAP[pair.first].c_str());
            ctx.writef("c%d.min 0", pair.first);
            ctx.writef("c%d.draw AREASTACK", pair.first);
            ctx.writef("c%d.info Number servers on the master belonging to %s", pair.first, SERVER_CHAIN_MAP[pair.first].c_str());
        }
    }

    if (ctx.isFetch()) {
        for (const auto &pair : map) {
            ctx.writef("c%d.value %d", pair.first, pair.second);
        }
    }
}