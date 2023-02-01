#include <cstdio>
#include <cstring>
#include "muninnode.h"
#include "util.h"
#include "muninregistration.h"
#include "main.h"

MuninNode::MuninNode(App *app) :
    app(app), server(INVALID_SOCKET), client(INVALID_SOCKET), useDirtyConfig(false) {
    int err;

    server = socket(AF_INET, SOCK_STREAM, 0);
    if (server == INVALID_SOCKET) {
        socket_perror("socket");
    }

    sockaddr_in serverAddr = { 0 };
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(14949);

    char yes = 1;
    err = setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
    if (err == SOCKET_ERROR) {
        socket_perror("setsockopt");
    }

    err = bind(server, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
    if (err == SOCKET_ERROR) {
        socket_perror("bind");
    }

    err = listen(server, 10);
    if (err == SOCKET_ERROR) {
        socket_perror("listen");
    }

    printf("Munin node is now listening.\n");
}

void MuninNode::run() {
    if (client == INVALID_SOCKET) {
        acceptClient();
    } else {
        doRead();
    }
}

void MuninNode::acceptClient() {
    sockaddr_storage client_addr{};
    socklen_t addr_size = sizeof(client_addr);

    client = accept(server, (struct sockaddr *)&client_addr, &addr_size);
    if (client == INVALID_SOCKET) {
        socket_perror("accept");
        return;
    }

    auto *sin = (struct sockaddr_in*)&client_addr;

    auto *ip = (unsigned char *)&sin->sin_addr.s_addr;
    printf("Incoming connection from: %d.%d.%d.%d\n", ip[0], ip[1], ip[2], ip[3]);
    send("# munin node at zandronum");
}

void MuninNode::doRead() {
    constexpr size_t bufferSize = 512;
    char buffer[bufferSize];
    memset(buffer, 0, bufferSize);

    int len = recv(client, buffer, bufferSize, 0);
    if (len < 1) {
        socket_perror("recv");
        closesocket(client);
        client = INVALID_SOCKET;
        return;
    } else {
        std::string data(buffer);
        std::vector<std::string> lines = split(data, '\n');

        for (auto &line : lines) {
            if (line[line.length() - 1] == '\r') {
                line.pop_back();
            }

            if (line.empty()) {
                continue;
            }

            std::vector<std::string> words = split(line, ' ');

            printf("%s\n", line.c_str());

            if (words[0] == "cap") {
                send("cap dirtyconfig");
                if (std::find(words.cbegin(), words.cend(), "dirtyconfig") != words.cend()) {
                    useDirtyConfig = true;
                }
            }
            else if (words[0] == "nodes") {
                send("zandronum");
                send(".");
            }
            else if (words[0] == "version") {
                send("zandrospy");
            }
            else if (words[0] == "list") {
                std::stringstream ss;
                const auto &plugins = app->getMuninPlugins();

                for (size_t i = 0; i < plugins.size(); i++) {
                    const auto &plugin = plugins[i];
                    ss << plugin->getName();

                    if (i < plugins.size() - 1) {
                        ss << " ";
                    }
                }

                send(ss.str());
            }
            else if (words[0] == "config") {
                std::string wanted;
                if (words.size() >= 2) { wanted = words[1]; }
                auto plug = MuninPlugin::findByName(wanted);
                if (plug == nullptr) {
                    send("# Unknown service");
                } else {
                    ExecutionContext ctx{app, this};
                    plug->config(ctx);
                    if (useDirtyConfig) {
                        plug->fetch(ctx);
                    }
                }
                send(".");
            }
            else if (words[0] == "fetch") {
                std::string wanted;
                if (words.size() >= 2) { wanted = words[1]; }
                auto plug = MuninPlugin::findByName(wanted);
                if (plug == nullptr) {
                    send("# Unknown service");
                } else {
                    ExecutionContext ctx{app, this};
                    plug->fetch(ctx);
                }
                send(".");
            }
            else if (words[0] == "quit" || words[0] == ".") {
                closesocket(client);
                client = INVALID_SOCKET;
                return;
            }
            else {
                send("# Unknown command. Try cap, list, nodes, config, fetch, version or quit");
            }
        }
    }
}

void MuninNode::send(const std::string &line) {
    std::string data = line + "\r\n";
    int r = ::send(client, data.c_str(), data.size(), 0);
    if (r != data.size()) {
        socket_perror("send");
    }
}

MuninNode::~MuninNode() {
    closesocket(server);
}