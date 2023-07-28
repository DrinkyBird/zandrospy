#include <cstdio>
#include <sstream>
#include <pthread.h>
#include <cstring>
#include "zanquerent.h"
#include "zanproto.h"
#include "buffer.h"
#include "zanserver.h"
#include "main.h"

static constexpr uint16_t MASTER_PORT = 15300;

static constexpr uint32_t QUERY_1 = SQF_PWADS|SQF_GAMETYPE|SQF_IWAD|SQF_NUMPLAYERS|SQF_PLAYERDATA|SQF_TEAMINFO_NUMBER|SQF_EXTENDED_INFO;

static constexpr uint32_t QUERY_2 = 0;

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
    if (time(nullptr) - lastQueryTime > 200) {
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
    stagingStats = {};
    queryStartTime = MeasureClock::now();

    r = sendto(socket, (const char *)buffer.getData(), (int)buffer.getLength(), 0, (const sockaddr *)&master_addr, sizeof(master_addr));
    if (r == SOCKET_ERROR) {
        socket_perror("sendto");
        return;
    }

    stagingStats.masterTrafficOut += buffer.getLength();
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
#ifdef _WIN32
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
#else
    timeout.tv_sec = 0;
    timeout.tv_usec = 500000;
#endif

    setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout, sizeof(timeout));

    sockaddr_in origin_addr{};
    socklen_t origin_len = sizeof(origin_addr);
    Buffer x(4096);
    int r = recvfrom(socket, (char *)x.getData(), (int)x.getLength(), 0, (sockaddr *)&origin_addr, &origin_len);
    if (r == SOCKET_ERROR ) {
        int err = socket_error();
        if (err != WSAETIMEDOUT && err != EAGAIN && err != EWOULDBLOCK) {
            socket_perror("recvfrom");
        }
        return;
    }
    x.resize(r);
    x.dehuffmanify();

    if (memcmp(&origin_addr.sin_addr, &masterAddr, masterAddrLen) == 0 && origin_addr.sin_port == htons(MASTER_PORT)) {
        stagingStats.masterTrafficIn += x.getLength();
        handleMasterResponse(x);
    } else {
        stagingStats.queryTrafficIn += x.getLength();
        handleServerResponse(x, origin_addr);
    }
}

void ZanQuerent::handleMasterResponse(Buffer &buffer) {
    while (!buffer.isEnd()) {
        auto response = buffer.read<uint32_t>();

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

                if (marker == MSC_SERVERBLOCK) {
                    uint8_t ports;
                    while ((ports = buffer.read<uint8_t>())) {
                        sockaddr_in addr{};
                        addr.sin_family = AF_INET;
                        addr.sin_addr.s_addr = buffer.read<uint32_t>();

                        for (int i = 0; i < ports; i++) {
                            addr.sin_port = htons(buffer.read<uint16_t>());
                            serverAddresses.emplace_back(addr);
                            queryQueue.push(addr);
                        }
                    }
                } else if (marker == MSC_SERVER) {
                    sockaddr_in addr{};
                    addr.sin_family = AF_INET;
                    addr.sin_addr.s_addr = buffer.read<uint32_t>();
                    addr.sin_port = htons(buffer.read<uint16_t>());
                    serverAddresses.emplace_back(addr);
                    queryQueue.push(addr);
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

    Buffer queryBuf(17);
    queryBuf.write<uint32_t>(199);
    queryBuf.write<uint32_t>(QUERY_1);
    queryBuf.write<uint32_t>(time(nullptr));
    queryBuf.write<uint32_t>(QUERY_2);
    queryBuf.write<uint8_t>(1);
    queryBuf.huffmanify();

    auto front = queryQueue.front();
    queryQueue.pop();

    std::string id = makeServerId(front);
    ZanServer server;
    stagingData[id] = server;

    int r = sendto(socket, (const char *)queryBuf.getData(), (int)queryBuf.getLength(), 0, (const sockaddr *)&front, sizeof(front));
    if (r == SOCKET_ERROR) {
        socket_perror("sendto");
        return;
    }

    stagingStats.queryTrafficOut += queryBuf.getLength();

    printf("working: %zu, %zu\n", queryQueue.size(), stagingData.size());

    if (queryQueue.empty() && !stagingData.empty()) {
        stagingStats.queryTime = toSeconds<MeasureClock>(MeasureClock::now() - queryStartTime);

        pthread_mutex_lock(&app->serverDataMutex);
        app->serverData.clear();
        for (const auto &pair : stagingData) {
            app->serverData[pair.first] = pair.second;
        }
        pthread_mutex_unlock(&app->serverDataMutex);

        pthread_mutex_lock(&app->queryStatsMutex);
        app->queryStats = stagingStats;
        pthread_mutex_unlock(&app->queryStatsMutex);

        printf("Refresh completed, data swapped.\n");
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

    bool segmented = false;

    server.response = buffer.read<int32_t>();
    printf("%u\n", server.response);

    if (server.response == SERVER_LAUNCHER_CHALLENGE) {
        auto pingTime = buffer.read<uint32_t>();
        server.version = buffer.readString();

        handleFields(server, buffer, false);
    } else if (server.response == SERVER_LAUNCHER_SEGMENTED_CHALLENGE) {
        unsigned int segmentNumber = buffer.read<uint8_t>();
        const int size = buffer.read<uint16_t>();

        bool isEnd = (segmentNumber & (1 << 7));
        segmentNumber &= ~(1 << 7);

        if (segmentNumber == 0) {
            auto pingTime = buffer.read<uint32_t>();
            server.version = buffer.readString();
        }

        handleFields(server, buffer, true);
    } else {
        return;
    }

    server.serverChain = determineServerChain(server, origin);
}

void ZanQuerent::handleFields(ZanServer &server, Buffer &buffer, bool segmented) {
    int fieldsetNum = -1;
    uint32_t flags = 0;
    while (!buffer.isEnd()) {
        fieldsetNum = segmented ? buffer.read<uint8_t>() : fieldsetNum + 1;
        flags = buffer.read<uint32_t>();

        if (fieldsetNum == 0) {
            server.flags = flags;

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
                int num = buffer.read<uint8_t>();
                server.players.resize(num);
                printf("%d players\n", num);
            }

            if (server.flags & SQF_PLAYERDATA) {
                bool isTeam = segmented ? !!buffer.read<uint8_t>() : (server.flags & SQF_TEAMINFO_NUMBER);
                for (int i = 0; i < server.players.size(); i++) {
                    ZanPlayer player;
                    player.name = buffer.readString();
                    player.score = buffer.read<uint16_t>();
                    player.ping = buffer.read<uint16_t>();
                    player.spectator = buffer.read<uint8_t>() != 0;
                    player.bot = buffer.read<uint8_t>() != 0;
                    if (isTeam) {
                        player.team = buffer.read<int8_t>();
                    } else {
                        player.team = -1;
                    }
                    player.time = buffer.read<int8_t>();
                    server.players[i] = player;

                    if (!player.bot) {
                        server.numHumanPlayers++;

                        if (player.spectator) {
                            server.numSpectators++;
                        } else {
                            server.numInGamePlayers++;
                        }
                    }
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
        } else if (fieldsetNum == 1) {
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

            if (server.flags2 & SQF2_GAMEMODE_NAME) {
                buffer.readString();
            }

            if (server.flags2 & SQF2_GAMEMODE_SHORTNAME) {
                buffer.readString();
            }
        }

        if (!(flags & SQF_EXTENDED_INFO)) {
            break;
        }
    }
}

int ZanQuerent::determineServerChain(ZanServer &server, const sockaddr_in &origin) {
    std::string addr = inet_ntoa(origin.sin_addr);

    if (addr == "104.128.58.120") {
        return ServerChain_TSPG;
    }
    else if (addr == "103.25.59.27") {
        return ServerChain_DUD;
    }
    else if (addr == "54.36.165.167") {
        return ServerChain_BlueFirestick;
    }
    else if (addr == "142.132.155.163") {
        return ServerChain_Euroboros;
    }
    else if (addr == "134.195.14.136") {
        return ServerChain_FAP;
    }
    else if (addr == "162.248.95.73") {
        return ServerChain_Rampage;
    }
    else if (addr == "185.150.189.56") {
        return ServerChain_DemonCrusher;
    }
    else if (addr == "192.155.90.89") {
        return ServerChain_IFOC;
    }
    else if (addr == "69.195.128.234") {
        return ServerChain_Dogsoft;
    }
    else if (addr == "79.174.15.140") {
        return ServerChain_IDDQD;
    }
    else if (addr == "81.162.236.129") {
        return ServerChain_Ninferno;
    }

    return ServerChain_Other;
}