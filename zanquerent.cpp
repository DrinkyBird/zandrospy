#include <cstdio>
#include <sstream>
#include <pthread.h>
#include "zanquerent.h"
#include "zanproto.h"
#include "buffer.h"
#include "zanserver.h"
#include "main.h"

static constexpr uint16_t MASTER_PORT = 15300;


ZanQuerent::ZanQuerent(App *app) :
    app(app),
    socket(INVALID_SOCKET), lastQueryTime(0) {
    socket = ::socket(AF_INET, SOCK_DGRAM, 0);

    hostent *he;
    in_addr **addrList;

    if ((he = gethostbyname("master.zandronum.com")) == nullptr) {
        printf("can't resolve hostname\n");
        return;
    }

    if (*he->h_addr_list == nullptr) {
        printf("no addresses");
        return;
    }

    memcpy(&masterAddr, he->h_addr_list[0], he->h_length);
    masterAddrLen = he->h_length;
}

ZanQuerent::~ZanQuerent() {
    closesocket(socket);
}

void ZanQuerent::run() {
    if (time(nullptr) - lastQueryTime > 20) {
        queryMaster();
        lastQueryTime = time(nullptr);
    }

    receive();
    workQueryQueue();
}

void ZanQuerent::queryMaster() {
    int r;

    sockaddr_in master_addr{};
    master_addr.sin_family = AF_INET;
    memcpy(&master_addr.sin_addr, &masterAddr, masterAddrLen);
    master_addr.sin_port = htons(MASTER_PORT);

    Buffer buffer(6);
    buffer.write<uint32_t>(LAUNCHER_MASTER_CHALLENGE);
    buffer.write<uint16_t>(2);
    buffer.huffmanify();

    queryQueue = {};
    serverAddresses.clear();
    stagingData.clear();

    r = sendto(socket, (const char *)buffer.getData(), (int)buffer.getLength(), 0, (const sockaddr *)&master_addr, sizeof(master_addr));
    if (r == SOCKET_ERROR) {
        socket_perror("sendto");
        return;
    }

}

std::string ZanQuerent::makeServerId(const sockaddr_in &origin) {
    std::stringstream ss;
    ss << inet_ntoa(origin.sin_addr);
    ss << ":";
    ss << ntohs(origin.sin_port);
    return ss.str();
}

void ZanQuerent::receive() {
    timeval timeout{};
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;

    setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout, sizeof(timeout));

    sockaddr_in origin_addr{};
    int origin_len = sizeof(origin_addr);
    Buffer x(4096);
    int r = recvfrom(socket, (char *)x.getData(), (int)x.getLength(), 0, (sockaddr *)&origin_addr, &origin_len);
    if (r == SOCKET_ERROR) {
        if (socket_error() != WSAETIMEDOUT) {
            socket_perror("recvfrom");
        }
        return;
    }
    //printf("Recv %d from %s:%d\n", r, inet_ntoa(origin_addr.sin_addr), ntohs(origin_addr.sin_port));
    x.resize(r);
    x.dehuffmanify();

    char fn[512];
    snprintf(fn, 512, "spam/data.%s.%d.bin", inet_ntoa(origin_addr.sin_addr), ntohs(origin_addr.sin_port));
    x.save(fn);

    if (memcmp(&origin_addr.sin_addr, &masterAddr, masterAddrLen) == 0 && origin_addr.sin_port == htons(MASTER_PORT)) {
        handleMasterResponse(x);
    } else {
        handleServerResponse(x, origin_addr);
    }
}

void ZanQuerent::handleMasterResponse(Buffer &buffer) {
    while (buffer.tell() < buffer.getLength()) {
        auto response = buffer.read<uint32_t>();
        printf("%d\n", response);

        switch (response) {
            case MSC_IPISBANNED: {
                printf("We're banned from the master apparently\n");
                break;
            }

            case MSC_REQUESTIGNORED: {
                printf("We're spamming the master and were ignored.\n");
                break;
            }

            case MSC_WRONGVERSION: {
                printf("We're speaking an old version of the master protocol.\n");
                break;
            }

            case MSC_BEGINSERVERLISTPART: {
                auto packet = buffer.read<uint8_t>();
                auto marker = buffer.read<uint8_t>();
                printf("marker = %d\n" ,marker);

                if (marker == MSC_SERVERBLOCK) {
                    while (true) {
                        auto num = buffer.read<uint8_t>();

                        if (num == 0) {
                            break;
                        }

                        sockaddr_in addr{};
                        addr.sin_family = AF_INET;
                        addr.sin_addr.S_un.S_un_b.s_b1 = buffer.read<uint8_t>();
                        addr.sin_addr.S_un.S_un_b.s_b2 = buffer.read<uint8_t>();
                        addr.sin_addr.S_un.S_un_b.s_b3 = buffer.read<uint8_t>();
                        addr.sin_addr.S_un.S_un_b.s_b4 = buffer.read<uint8_t>();

                        printf("%s\n", inet_ntoa(addr.sin_addr));

                        for (int i = 0; i < num; i++) {
                            addr.sin_port = htons(buffer.read<uint16_t>());
                            printf("    : %d\n", ntohs(addr.sin_port));
                            serverAddresses.emplace_back(addr);
                            queryQueue.push(addr);
                        }
                    }
                }

                break;
            }

            case MSC_ENDSERVERLIST:
            case MSC_ENDSERVERLISTPART: {
                break;
            }

            default: {
                printf("Invalid response %d.\n", response);
                break;
            }
        }
    }
}

void ZanQuerent::workQueryQueue() {
    if (queryQueue.empty()) {
        return;
    }

    Buffer queryBuf(16);
    queryBuf.write<uint32_t>(199);
    queryBuf.write<uint32_t>(SQF_ALL);
    queryBuf.write<uint32_t>(0);
    queryBuf.write<uint32_t>(SQF2_ALL);
    queryBuf.huffmanify();

    auto front = queryQueue.front();
    queryQueue.pop();

    std::string id = makeServerId(front);
    ZanServer server;
    stagingData[id] = server;

    printf("Querying: %s:%d\n", inet_ntoa(front.sin_addr), ntohs(front.sin_port));
    int r = sendto(socket, (const char *)queryBuf.getData(), (int)queryBuf.getLength(), 0, (const sockaddr *)&front, sizeof(front));
    if (r == SOCKET_ERROR) {
        socket_perror("sendto");
        return;
    }

    if (queryQueue.empty() && !stagingData.empty()) {
        pthread_mutex_lock(&app->serverDataMutex);
        app->serverData.clear();
        for (const auto &pair : stagingData) {
            app->serverData[pair.first] = pair.second;
        }
        pthread_mutex_unlock(&app->serverDataMutex);
        stagingData.clear();
    }
}

void ZanQuerent::handleServerResponse(Buffer &buffer, const sockaddr_in &origin) {
    std::string id = makeServerId(origin);
    if (!stagingData.count(id)) {
        ZanServer s;
        stagingData[id] = s;
    }

    ZanServer &server = stagingData[id];

    server.response = buffer.read<int32_t>();
    auto pingTime = buffer.read<uint32_t>();

    printf("Response from %s:%d %s\n", inet_ntoa(origin.sin_addr), ntohs(origin.sin_port), id.c_str());

    if (server.response != 5660023) {
        switch (server.response) {
            case 5660024: {
                printf("Server ignored us for spamming\n");
                break;
            }

            case 5660025: {
                printf("Server banned us\n");
                break;
            }
        }

        return;
    }

    server.version = buffer.readString();
    server.flags = buffer.read<uint32_t>();
    int numPlayers = 0;

    printf("    Version: %s\n", server.version.c_str());
    printf("    Flags: %08x\n", server.flags);

    if (server.flags & SQF_NAME) {
        server.name = buffer.readString();
    }

    if (server.flags & SQF_URL) {
        server.url = buffer.readString();
    }

    if (server.flags & SQF_EMAIL) {
        server.email = buffer.readString();
    }

    if (server.flags & SQF_MAPNAME) {
        server.mapname = buffer.readString();
    }

    if (server.flags & SQF_MAXCLIENTS) {
        server.maxClients = buffer.read<uint8_t>();
    }

    if (server.flags & SQF_MAXPLAYERS) {
        server.maxPlayers = buffer.read<uint8_t>();
    }

    if (server.flags & SQF_PWADS) {
        auto num = buffer.read<uint8_t>();
        for (int i = 0; i < num; i++) {
            ZanPWAD pwad;
            pwad.name = buffer.readString();
            pwad.isOptional = false;
            pwad.hash = "";

            server.pwads.emplace_back(pwad);
        }
    }

    if (server.flags & SQF_GAMETYPE) {
        server.gamemode = buffer.read<uint8_t>();
        server.instagib = buffer.read<uint8_t>() != 0;
        server.buckshot = buffer.read<uint8_t>() != 0;
    }

    if (server.flags & SQF_GAMENAME) {
        buffer.readString();
    }

    if (server.flags & SQF_IWAD) {
        server.iwad = buffer.readString();
    }

    if (server.flags & SQF_FORCEPASSWORD) {
        server.forcePassword = buffer.read<uint8_t>() != 0;
    }

    if (server.flags & SQF_FORCEJOINPASSWORD) {
        server.forceJoinPassword = buffer.read<uint8_t>() != 0;
    }

    if (server.flags & SQF_GAMESKILL) {
        server.skill = buffer.read<uint8_t>();
    }

    if (server.flags & SQF_BOTSKILL) {
        server.botSkill = buffer.read<uint8_t>();
    }

    if (server.flags & SQF_DMFLAGS) {
        server.dmflags = buffer.read<uint32_t>();
        server.dmflags2 = buffer.read<uint32_t>();
        server.compatflags = buffer.read<uint32_t>();
    }

    if (server.flags & SQF_LIMITS) {
        server.fragLimit = buffer.read<uint16_t>();
        server.timeLimit = buffer.read<uint16_t>();
        server.timeLeft = server.timeLimit > 0 ? buffer.read<uint16_t>() : 0;
        server.duelLimit = buffer.read<uint16_t>();
        server.pointLimit = buffer.read<uint16_t>();
        server.winLimit = buffer.read<uint16_t>();
    }

    if (server.flags & SQF_TEAMDAMAGE) {
        server.teamDamage = buffer.read<float>();
    }

    if (server.flags & SQF_TEAMSCORES) {
        buffer.read<uint16_t>();
        buffer.read<uint16_t>();
    }

    if (server.flags & SQF_NUMPLAYERS) {
        numPlayers = buffer.read<uint8_t>();
    }

    if (server.flags & SQF_PLAYERDATA) {
        for (int i = 0; i < numPlayers; i++) {
            ZanPlayer player;
            player.name = buffer.readString();
            player.score = buffer.read<uint16_t>();
            player.ping = buffer.read<uint16_t>();
            player.spectator = buffer.read<uint8_t>() != 0;
            player.bot = buffer.read<uint8_t>() != 0;
            if (server.flags & SQF_TEAMINFO_NUMBER) {
                player.team = buffer.read<int8_t>();
            } else {
                player.team = -1;
            }
            player.time = buffer.read<int8_t>();
            server.players.emplace_back(player);
        }
    }

    int numTeams = 0;
    if (server.flags & SQF_TEAMINFO_NUMBER) {
        numTeams = buffer.read<uint8_t>();
        for (int i = 0; i < numTeams; i++) {
            ZanTeam team;
            server.teams.emplace_back(team);
        }
    }

    if (server.flags & SQF_TEAMINFO_NAME) {
        for (int i = 0; i < numTeams; i++) {
            server.teams[i].name = buffer.readString();
        }
    }

    if (server.flags & SQF_TEAMINFO_COLOR) {
        for (int i = 0; i < numTeams; i++) {
            server.teams[i].colour = buffer.read<uint32_t>();
        }
    }

    if (server.flags & SQF_TEAMINFO_SCORE) {
        for (int i = 0; i < numTeams; i++) {
            server.teams[i].score = buffer.read<uint16_t>();
        }
    }

    if (server.flags & SQF_TESTING_SERVER) {
        server.isTestingBuild = buffer.read<uint8_t>() != 0;
        server.testBuildUrl = buffer.readString();
    }

    if (server.flags & SQF_DATA_MD5SUM) {
        buffer.readString();
    }

    if (server.flags & SQF_ALL_DMFLAGS) {
        int num = buffer.read<uint8_t>();
        for (int i = 0; i < num; i++) {
            auto val = buffer.read<uint32_t>();
            if (i == 0) server.dmflags = val;
            if (i == 1) server.dmflags2 = val;
            if (i == 2) server.zadmflags = val;
            if (i == 3) server.compatflags = val;
            if (i == 4) server.zacompatflags = val;
            if (i == 5) server.compatflags2 = val;
        }
    }

    if (server.flags & SQF_SECURITY_SETTINGS) {
        server.securitySettings = buffer.read<uint8_t>() != 0;
    }

    if (server.flags & SQF_OPTIONAL_WADS) {
        int num = buffer.read<uint8_t>();
        for (int i = 0; i < num; i++) {
            server.pwads[buffer.read<uint8_t>()].isOptional = true;
        }
    }

    if (server.flags & SQF_DEH) {
        int num = buffer.read<uint8_t>();
        for (int i = 0; i < num; i++) {
            server.dehackedPatches.emplace_back(buffer.readString());
        }
    }

    if (server.flags & SQF_EXTENDED_INFO) {
        server.flags2 = buffer.read<uint32_t>();

        if (server.flags2 & SQF2_PWAD_HASHES) {
            int num = buffer.read<uint8_t>();
            for (int i = 0; i < num; i++) {
                server.pwads[i].hash = buffer.readString();
            }
        }

        if (server.flags2 & SQF2_COUNTRY) {
            server.country.clear();
            server.country += buffer.read<char>();
            server.country += buffer.read<char>();
            server.country += buffer.read<char>();
        }
    }
}