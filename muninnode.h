#pragma once
#include <string>
#include "socket.h"
#include "util.h"

class App;

class MuninNode {
public:
    MuninNode(App *app);
    ~MuninNode();

    void run();
    void doRead();

    void send(const std::string &line);

    double getLastConnectionTime() const;

private:
    void acceptClient();
    void endConnection();

    App *app;

    SOCKET server;
    SOCKET client;

    bool useDirtyConfig;
    double lastConnectionTime;
    MeasureClock::time_point connectionStartTime;
};
