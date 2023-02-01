#pragma once
#include <string>
#include <cstdint>
#include <vector>

struct ZanPWAD {
    ZanPWAD() : isOptional(false) {}

    std::string name;
    bool isOptional;
    std::string hash;
};

struct ZanPlayer {
    ZanPlayer() : score(0), ping(0), spectator(false), bot(false), team(-1), time(0) {}

    std::string name;
    int score;
    int ping;
    bool spectator;
    bool bot;
    int team;
    int time;
};

struct ZanTeam {
    ZanTeam() : colour(0), score(0) {}

    std::string name;
    uint32_t colour;
    int score;
};

struct ZanServer {
    ZanServer() :
        response(-1), flags(0), flags2(0),
        maxClients(0), maxPlayers(0), pwads(),
        gamemode(0), instagib(false), buckshot(false),
        forcePassword(false), forceJoinPassword(false),
        skill(0), botSkill(-1), fragLimit(0), timeLimit(0), timeLeft(0), duelLimit(0), pointLimit(0), winLimit(0),
        teamDamage(0.0f), players(), teams(), isTestingBuild(false),
        dmflags(0), dmflags2(0), zadmflags(0), compatflags(0), zacompatflags(0), compatflags2(0),
        dehackedPatches(), country("XUN") {}

    int32_t response;
    [[nodiscard]] constexpr inline bool success() const { return response == 5660023; }

    std::string version;

    uint32_t flags, flags2;

    std::string name;
    std::string url;
    std::string email;
    std::string mapname;
    int maxClients;
    int maxPlayers;
    std::vector<ZanPWAD> pwads;
    int gamemode;
    bool instagib;
    bool buckshot;
    std::string iwad;
    bool forcePassword;
    bool forceJoinPassword;
    int skill;
    int botSkill;
    int fragLimit;
    int timeLimit;
    int timeLeft;
    int duelLimit;
    int pointLimit;
    int winLimit;
    float teamDamage;
    std::vector<ZanPlayer> players;
    std::vector<ZanTeam> teams;
    bool isTestingBuild;
    std::string testBuildUrl;
    uint32_t dmflags, dmflags2, zadmflags, compatflags, zacompatflags, compatflags2;
    bool securitySettings{};
    std::vector<std::string> dehackedPatches;
    std::string country;
};