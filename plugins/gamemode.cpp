#include <string_view>
#include <map>
#include "muninregistration.h"
#include "util.h"

static constexpr int GAMEMODE_COUNT = 16;
static constexpr const char *GAMEMODE_NAMES[GAMEMODE_COUNT] = {
    "Cooperative",
    "Survival",
    "Invasion",
    "Deathmatch",
    "Team DM",
    "Duel",
    "Terminator",
    "Last Man Standing",
    "Team LMS",
    "Possession",
    "Team Possession",
    "Team Game",
    "CTF",
    "One-Flag CTF",
    "Skulltag",
    "Domination"
};

REGISTER_PLUGIN(gamemodes_servers) {
    std::map<int, int> map;
    for (int i = 0; i < GAMEMODE_COUNT; i++) {
        map[i] = 0;
    }

    if (ctx.isConfig()) {
        ctx.write("graph_title Game modes by server count");
        ctx.write("graph_category gamemode");

        for (const auto &pair : map) {
            ctx.writef("gm%d.label %s", pair.first, GAMEMODE_NAMES[pair.first]);
            ctx.writef("gm%d.min 0", pair.first);
            ctx.writef("gm%d.draw AREASTACK", pair.first);
        }
    }

    if (ctx.isFetch()) {
        ctx.lockServerData();
        for (const auto &pair : ctx.getServerData()) {
            const auto &server = pair.second;
            if (!server.success()) { continue; }

            if (map.count(server.gamemode)) {
                map[server.gamemode]++;
            }
        }
        ctx.unlockServerData();

        for (const auto &pair : map) {
            ctx.writef("gm%d.value %d", pair.first, pair.second);
        }
    }
}

REGISTER_PLUGIN(gamemodes_players) {
    std::map<int, int> map;
    for (int i = 0; i < GAMEMODE_COUNT; i++) {
        map[i] = 0;
    }

    if (ctx.isConfig()) {
        ctx.write("graph_title Game modes by player count");
        ctx.write("graph_category gamemode");

        for (const auto &pair : map) {
            ctx.writef("gm%d.label %s", pair.first, GAMEMODE_NAMES[pair.first]);
            ctx.writef("gm%d.min 0", pair.first);
            ctx.writef("gm%d.draw AREASTACK", pair.first);
        }
    }

    if (ctx.isFetch()) {
        ctx.lockServerData();
        for (const auto &pair : ctx.getServerData()) {
            const auto &server = pair.second;
            if (!server.success()) { continue; }

            if (map.count(server.gamemode)) {
                map[server.gamemode] += server.numHumanPlayers;
            }
        }
        ctx.unlockServerData();

        for (const auto &pair : map) {
            ctx.writef("gm%d.value %d", pair.first, pair.second);
        }
    }
}