#pragma once

#include <vector>
#include <queue>
#include <unordered_map>
#include <string>
#include <ctime>
#include <memory>
#include "socket.h"
#include "zanserver.h"
#include "querystats.h"
#include "util.h"

class App;
class Buffer;

struct SegmentStorage {
    std::unique_ptr<Buffer> buffer;
    int segmentsReceived;
};

class ZanQuerent {
public:
    ZanQuerent(App *app);
    ~ZanQuerent();

    void run();

    void queryMaster();
    void workQueryQueue();
    void swapServers();

private:
    std::string makeServerId(const sockaddr_in &origin);

    void receive();
    void handleMasterResponse(Buffer &buffer);
    void handleServerResponse(Buffer &buffer, const sockaddr_in &origin);
    void handleFullResponse(ZanServer &server, Buffer &buffer);
    int determineServerChain(ZanServer &server, const sockaddr_in &origin);

    App *app;

    SOCKET socket;
    sockaddr_in masterAddr;
    int masterAddrLen;

    std::vector<sockaddr_in> serverAddresses;
    std::queue<sockaddr_in> queryQueue;

    std::unordered_map<std::string, ZanServer> stagingData;
    std::unordered_map<std::string, SegmentStorage> segmentedBuffers;
    QueryStats stagingStats;
    MeasureClock::time_point queryStartTime;

    time_t lastQueryTime, lastSendTime;
};
