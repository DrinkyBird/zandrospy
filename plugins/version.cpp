#include <sstream>
#include "muninregistration.h"
#include "util.h"

static std::string filterVersion(const std::string &ver) {
    std::stringstream ss;
    for (const char &c : ver) {
        if (!std::isalnum(c)) {
            ss << '_';
        } else {
            ss << c;
        }
    }
    return ss.str();
}

REGISTER_PLUGIN(versions_servers) {
    std::unordered_map<std::string, int> map;
    ctx.lockServerData();
    for (const auto &pair : ctx.getServerData()) {
        const auto &server = pair.second;
        if (!server.success()) continue;

        auto words = split(server.version, ' ');
        if (words.empty()) continue;

        auto version = words[0];
        if (map.count(version) == 0) {
            map[version] = 1;
        } else {
            map[version]++;
        }
    }
    ctx.unlockServerData();

    if (ctx.isConfig()) {
        ctx.write("graph_title Versions by server count");
        ctx.write("graph_category players");

        for (const auto &pair : map) {
            auto filtered = filterVersion(pair.first);
            ctx.writef("%s.label %s", filtered.c_str(), pair.first.c_str());
            ctx.writef("%s.min 0", filtered.c_str());
        }
    }

    if (ctx.isFetch()) {
        for (const auto &pair : map) {
            auto filtered = filterVersion(pair.first);
            ctx.writef("%s.value %d", filtered.c_str(), pair.second);
        }
    }
}


REGISTER_PLUGIN(versions_players) {
    std::unordered_map<std::string, int> map;
    ctx.lockServerData();
    for (const auto &pair : ctx.getServerData()) {
        const auto &server = pair.second;
        if (!server.success()) continue;

        auto words = split(server.version, ' ');
        if (words.empty()) continue;

        auto version = words[0];
        if (map.count(version) == 0) {
            map[version] = server.numHumanPlayers;
        } else {
            map[version] += server.numHumanPlayers;
        }
    }
    ctx.unlockServerData();

    if (ctx.isConfig()) {
        ctx.write("graph_title Versions by player count");
        ctx.write("graph_category players");

        for (const auto &pair : map) {
            auto filtered = filterVersion(pair.first);
            ctx.writef("%s.label %s", filtered.c_str(), pair.first.c_str());
            ctx.writef("%s.min 0", filtered.c_str());
        }
    }

    if (ctx.isFetch()) {
        for (const auto &pair : map) {
            auto filtered = filterVersion(pair.first);
            ctx.writef("%s.value %d", filtered.c_str(), pair.second);
        }
    }
}
