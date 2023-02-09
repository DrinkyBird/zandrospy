#include <sstream>
#include <regex>
#include "muninregistration.h"
#include "util.h"

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
        ctx.write("graph_category version");
        ctx.write("graph_vlabel Servers");

        for (const auto &pair : map) {
            auto filtered = filterKey(pair.first);
            ctx.writef("%s.label %s", filtered.c_str(), pair.first.c_str());
            ctx.writef("%s.min 0", filtered.c_str());
            ctx.writef("%s.draw AREASTACK", filtered.c_str());
            ctx.writef("%s.info Number of servers on the master running Zandronum %s", filtered.c_str(), pair.first.c_str());
        }
    }

    if (ctx.isFetch()) {
        for (const auto &pair : map) {
            auto filtered = filterKey(pair.first);
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
        ctx.write("graph_title Versions by client count");
        ctx.write("graph_category version");
        ctx.write("graph_vlabel Players");

        for (const auto &pair : map) {
            auto filtered = filterKey(pair.first);
            ctx.writef("%s.label %s", filtered.c_str(), pair.first.c_str());
            ctx.writef("%s.min 0", filtered.c_str());
            ctx.writef("%s.draw AREASTACK", filtered.c_str());
            ctx.writef("%s.info Number of human (non-bot) clients on servers running Zandronum %s", filtered.c_str(), pair.first.c_str());
        }
    }

    if (ctx.isFetch()) {
        for (const auto &pair : map) {
            auto filtered = filterKey(pair.first);
            ctx.writef("%s.value %d", filtered.c_str(), pair.second);
        }
    }
}

REGISTER_PLUGIN(platforms_servers) {
    const std::regex re{ R"( on (.*)$)" };

    std::unordered_map<std::string, int> map;
    ctx.lockServerData();
    for (const auto &pair : ctx.getServerData()) {
        const auto &server = pair.second;
        if (!server.success()) continue;

        std::smatch m;
        if (std::regex_search(server.version, m, re)) {
            auto match = m[1].str();
            if (map.count(match) == 0) {
                map[match] = 1;
            } else {
                map[match]++;
            }
        }
    }
    ctx.unlockServerData();

    if (ctx.isConfig()) {
        ctx.write("graph_title Platforms by server count");
        ctx.write("graph_category version");
        ctx.write("graph_vlabel Servers");

        for (const auto &pair : map) {
            auto filtered = filterKey(pair.first);
            ctx.writef("%s.label %s", filtered.c_str(), pair.first.c_str());
            ctx.writef("%s.min 0", filtered.c_str());
            ctx.writef("%s.draw AREASTACK", filtered.c_str());
            ctx.writef("%s.info Number of servers running %s", filtered.c_str(), pair.first.c_str());
        }
    }

    if (ctx.isFetch()) {
        for (const auto &pair : map) {
            auto filtered = filterKey(pair.first);
            ctx.writef("%s.value %d", filtered.c_str(), pair.second);
        }
    }
}
