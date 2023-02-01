#include <iostream>
#include <pthread.h>
#include "main.h"
#include "socket.h"
#include "muninnode.h"
#include "zanquerent.h"
#include "huffman/huffman.h"

static void *muninThreadFunc(void *data);

static void *zandroThreadFunc(void *data);

int main() {
    App *app = new App;
    app->run();
    delete app;

    return 0;
}

void *muninThreadFunc(void *data) {
    App *app = (App *)data;
    MuninNode *node = new MuninNode(app);

    while (true) {
        node->run();
    }

    delete node;

    return nullptr;
}

static void *zandroThreadFunc(void *data) {
    App *app = (App *)data;
    ZanQuerent *querent = new ZanQuerent(app);

    while (true) {
        querent->run();
    }

    delete querent;
    return nullptr;
}

App::App() {
#ifdef _WIN32
    setbuf(stdout, nullptr);
    setbuf(stderr, nullptr);

    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
    HUFFMAN_Construct();

    for (auto *reg = MuninPlugin::first; reg != nullptr; reg = reg->next) {
        muninPlugins.emplace_back(reg);
    }

    pthread_mutex_init(&serverDataMutex, nullptr);
}

App::~App() {
    pthread_mutex_destroy(&serverDataMutex);
    HUFFMAN_Destruct();
#ifdef _WIN32
    WSACleanup();
#endif
}

void App::run() {
    int r = pthread_create(&muninThread, nullptr, muninThreadFunc, this);
    r = pthread_create(&zandroThread, nullptr, zandroThreadFunc, this);

    pthread_join(muninThread, nullptr);
    pthread_join(zandroThread, nullptr);
}
