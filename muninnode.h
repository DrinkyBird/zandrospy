#pragma once
#include <string>

#include "socket.h"

class App;

class MuninNode {
public:
    MuninNode(App *app);
    ~MuninNode();

    void run();
    void doRead();

    void send(const std::string &line);

private:
    void acceptClient();

    App *app;

    SOCKET server;
    SOCKET client;

    bool useDirtyConfig;
};
