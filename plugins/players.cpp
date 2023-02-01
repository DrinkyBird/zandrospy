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