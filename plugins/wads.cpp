#include <map>
#include "muninregistration.h"
#include "util.h"

REGISTER_PLUGIN(iwads_servers) {
    std::map<std::string, int> map;

    ctx.lockServerData();
    for (const auto &pair : ctx.getServerData()) {
        const auto &server = pair.second;
        if (!server.success()) continue;

        std::string key = lowercase(server.iwad);

        if (map.count(key) == 0) {
            map[key] = 1;
        } else {
            map[key]++;
        }
    }
    ctx.unlockServerData();

    if (ctx.isConfig()) {
        ctx.write("graph_title IWADs by server count");
        ctx.write("graph_category wads");

        for (const auto &pair : map) {
            auto filtered = filterKey(pair.first);
            ctx.writef("%s.label %s", filtered.c_str(), pair.first.c_str());
            ctx.writef("%s.min 0", filtered.c_str());
            ctx.writef("%s.draw AREASTACK", filtered.c_str());
        }
    }

    if (ctx.isFetch()) {
        for (const auto &pair : map) {
            auto filtered = filterKey(pair.first);
            ctx.writef("%s.value %d", filtered.c_str(), pair.second);
        }
    }
}

REGISTER_PLUGIN(iwads_players) {
    std::map<std::string, int> map;

    ctx.lockServerData();
    for (const auto &pair : ctx.getServerData()) {
        const auto &server = pair.second;
        if (!server.success()) continue;

        std::string key = lowercase(server.iwad);

        if (map.count(key) == 0) {
            map[key] = server.numHumanPlayers;
        } else {
            map[key] += server.numHumanPlayers;
        }
    }
    ctx.unlockServerData();

    if (ctx.isConfig()) {
        ctx.write("graph_title IWADs by player count");
        ctx.write("graph_category wads");

        for (const auto &pair : map) {
            auto filtered = filterKey(pair.first);
            ctx.writef("%s.label %s", filtered.c_str(), pair.first.c_str());
            ctx.writef("%s.min 0", filtered.c_str());
            ctx.writef("%s.draw AREASTACK", filtered.c_str());
        }
    }

    if (ctx.isFetch()) {
        for (const auto &pair : map) {
            auto filtered = filterKey(pair.first);
            ctx.writef("%s.value %d", filtered.c_str(), pair.second);
        }
    }
}

REGISTER_PLUGIN(pwads_servers) {
    std::map<std::string, int> map;

    ctx.lockServerData();
    for (const auto &pair : ctx.getServerData()) {
        const auto &server = pair.second;
        if (!server.success()) continue;

        for (const auto &pwad : server.pwads) {
            std::string key = lowercase(pwad.name);

            if (map.count(key) == 0) {
                map[key] = 1;
            } else {
                map[key]++;
            }
        }
    }
    ctx.unlockServerData();

    if (ctx.isConfig()) {
        ctx.write("graph_title PWADs by server count");
        ctx.write("graph_category wads");

        for (const auto &pair : map) {
            auto filtered = filterKey(pair.first);
            ctx.writef("%s.label %s", filtered.c_str(), pair.first.c_str());
            ctx.writef("%s.min 0", filtered.c_str());
        }
    }

    if (ctx.isFetch()) {
        for (const auto &pair : map) {
            auto filtered = filterKey(pair.first);
            ctx.writef("%s.value %d", filtered.c_str(), pair.second);
        }
    }
}

REGISTER_PLUGIN(pwads_players) {
    std::map<std::string, int> map;

    ctx.lockServerData();
    for (const auto &pair : ctx.getServerData()) {
        const auto &server = pair.second;
        if (!server.success()) continue;

        for (const auto &pwad : server.pwads) {
            std::string key = lowercase(pwad.name);

            if (map.count(key) == 0) {
                map[key] = server.numHumanPlayers;
            } else {
                map[key] += server.numHumanPlayers;
            }
        }
    }
    ctx.unlockServerData();

    if (ctx.isConfig()) {
        ctx.write("graph_title PWADs by player count");
        ctx.write("graph_category wads");

        for (const auto &pair : map) {
            auto filtered = filterKey(pair.first);
            ctx.writef("%s.label %s", filtered.c_str(), pair.first.c_str());
            ctx.writef("%s.min 0", filtered.c_str());
        }
    }

    if (ctx.isFetch()) {
        for (const auto &pair : map) {
            auto filtered = filterKey(pair.first);
            ctx.writef("%s.value %d", filtered.c_str(), pair.second);
        }
    }
}