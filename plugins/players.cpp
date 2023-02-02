#include "muninregistration.h"

REGISTER_PLUGIN(players) {
    if (ctx.isConfig()) {
        ctx.write("graph_title Total players");
        ctx.write("graph_category players");
        ctx.write("players.label Players");
        ctx.write("players.min 0");
    }

    if (ctx.isFetch()) {
        int num = 0;

        ctx.lockServerData();
        for (const auto &pair : ctx.getServerData()) {
            if (pair.second.success()) {
                num += pair.second.numHumanPlayers;
            }
        }
        ctx.unlockServerData();

        ctx.writef("players.value %d", num);
    }
}

REGISTER_PLUGIN(players_chain) {
    std::unordered_map<int, int> map;
    for (const auto &pair : SERVER_CHAIN_MAP) {
        map[pair.first] = 0;
    }

    ctx.lockServerData();
    for (const auto &pair : ctx.getServerData()) {
        const auto &server = pair.second;
        map[server.serverChain] += server.numHumanPlayers;
    }
    ctx.unlockServerData();

    if (ctx.isConfig()) {
        ctx.write("graph_title Players by server chain");
        ctx.write("graph_category players");

        for (const auto &pair : map) {
            ctx.writef("c%d.label %s", pair.first, SERVER_CHAIN_MAP[pair.first].c_str());
            ctx.writef("c%d.min 0", pair.first);
            ctx.writef("c%d.draw AREASTACK", pair.first);
        }
    }

    if (ctx.isFetch()) {
        for (const auto &pair : map) {
            ctx.writef("c%d.value %d", pair.first, pair.second);
        }
    }
}