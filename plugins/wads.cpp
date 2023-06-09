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
        ctx.write("graph_vlabel Servers");

        for (const auto &pair : map) {
            auto filtered = filterKey(pair.first);
            ctx.writef("%s.label %s", filtered.c_str(), pair.first.c_str());
            ctx.writef("%s.min 0", filtered.c_str());
            ctx.writef("%s.draw AREASTACK", filtered.c_str());
            ctx.writef("%s.info Total number of servers on the master running %s", filtered.c_str(), pair.first.c_str());
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
        ctx.write("graph_title IWADs by client count");
        ctx.write("graph_category wads");
        ctx.write("graph_vlabel Players");

        for (const auto &pair : map) {
            auto filtered = filterKey(pair.first);
            ctx.writef("%s.label %s", filtered.c_str(), pair.first.c_str());
            ctx.writef("%s.min 0", filtered.c_str());
            ctx.writef("%s.draw AREASTACK", filtered.c_str());
            ctx.writef("%s.info Total number of human (non-bot) clients on servers running %s", filtered.c_str(), pair.first.c_str());
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
    std::vector<std::pair<std::string, int>> sorted;

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

    for (const auto &pair : map) {
        sorted.emplace_back(pair);
    }

    std::sort(sorted.begin(), sorted.end(), [](const auto &a, const auto &b) { return a.second > b.second; });

    std::string order;
    {
        std::stringstream ss;
        for (const auto &pair : sorted) {
            ss << pair.first << " ";
        }
        order = ss.str();
    }

    if (ctx.isConfig()) {
        ctx.write("graph_title PWADs by server count");
        ctx.write("graph_category wads");
        ctx.write("graph_vlabel Servers");
        ctx.writef<32 * 1024>("graph_order %s", order.c_str());

        for (const auto &pair : sorted) {
            auto filtered = filterKey(pair.first);
            ctx.writef("%s.label %s", filtered.c_str(), pair.first.c_str());
            ctx.writef("%s.min 0", filtered.c_str());
            ctx.writef("%s.info Total number of servers on the master running %s", filtered.c_str(), pair.first.c_str());
        }
    }

    if (ctx.isFetch()) {
        for (const auto &pair : sorted) {
            auto filtered = filterKey(pair.first);
            ctx.writef("%s.value %d", filtered.c_str(), pair.second);
        }
    }
}

REGISTER_PLUGIN(pwads_players) {
    std::map<std::string, int> map;
    std::vector<std::pair<std::string, int>> sorted;

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

    for (const auto &pair : map) {
        sorted.emplace_back(pair);
    }

    std::sort(sorted.begin(), sorted.end(), [](const auto &a, const auto &b) { return a.second > b.second; });

    std::string order;
    {
        std::stringstream ss;
        for (const auto &pair : sorted) {
            ss << pair.first << " ";
        }
        order = ss.str();
    }

    if (ctx.isConfig()) {
        ctx.write("graph_title PWADs by client count");
        ctx.write("graph_category wads");
        ctx.write("graph_vlabel Players");
        ctx.writef<32 * 1024>("graph_order %s", order.c_str());

        for (const auto &pair : sorted) {
            auto filtered = filterKey(pair.first);
            ctx.writef("%s.label %s", filtered.c_str(), pair.first.c_str());
            ctx.writef("%s.min 0", filtered.c_str());
            ctx.writef("%s.info Total number of human (non-bot) clients on servers running %s", filtered.c_str(), pair.first.c_str());
        }
    }

    if (ctx.isFetch()) {
        for (const auto &pair : sorted) {
            auto filtered = filterKey(pair.first);
            ctx.writef("%s.value %d", filtered.c_str(), pair.second);
        }
    }
}