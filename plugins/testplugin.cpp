#include "muninregistration.h"

REGISTER_PLUGIN(servers);

DEFINE_PLUGIN_CONFIG(servers) {
    ctx.write("graph_title Total servers");
    ctx.write("graph_category servers");
    ctx.write("servers.label Servers");
    ctx.write("servers.min 0");
}

DEFINE_PLUGIN_FETCH(servers) {
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

REGISTER_PLUGIN(players);

DEFINE_PLUGIN_CONFIG(players) {
    ctx.write("graph_title Total players");
    ctx.write("graph_category servers");
    ctx.write("servers.label Players");
    ctx.write("servers.min 0");
}

DEFINE_PLUGIN_FETCH(players) {
    int num = 0;

    ctx.lockServerData();
    for (const auto &pair : ctx.getServerData()) {
        if (pair.second.success()) {
            for (const auto &player : pair.second.players) {
                if (!player.bot) {
                    num++;
                }
            }
        }
    }
    ctx.unlockServerData();

    ctx.writef("players.value %d", num);
}