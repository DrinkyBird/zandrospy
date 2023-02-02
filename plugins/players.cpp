#include "muninregistration.h"
#include "util.h"

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

REGISTER_PLUGIN(players_state) {
    std::unordered_map<std::string, int> map = {
        { "In-Game", 0 },
        { "Spectator", 0 },
        { "Bot", 0 }
    };

    if (ctx.isConfig()) {
        ctx.write("graph_title Total players by state");
        ctx.write("graph_category players");
        for (const auto &pair : map) {
            const std::string key = lowercase(pair.first);
            ctx.writef("%s.label %s", key.c_str(), pair.first.c_str());
            ctx.writef("%s.min 0", key.c_str());
            ctx.writef("%s.draw AREASTACK", key.c_str());
        }
    }

    if (ctx.isFetch()) {
        ctx.lockServerData();
        for (const auto &pair : ctx.getServerData()) {
            if (pair.second.success()) {
                for (const auto &player : pair.second.players) {
                    if (player.bot) {
                        map["Bot"]++;
                    } else if (player.spectator) {
                        map["Spectator"]++;
                    } else {
                        map["In-Game"]++;
                    }
                }
            }
        }
        ctx.unlockServerData();

        for (const auto &pair : map) {
            const std::string key = lowercase(pair.first);
            ctx.writef("%s.value %d", key.c_str(), pair.second);
        }
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