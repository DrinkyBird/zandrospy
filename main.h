#pragma once
#include <pthread.h>
#include <unordered_map>
#include "muninregistration.h"
#include "zanserver.h"
#include "querystats.h"

class App {
public:
    App();
    ~App();

    void run();

    [[nodiscard]] const std::vector<MuninPlugin *> &getMuninPlugins() const { return muninPlugins; }

    mutable pthread_mutex_t serverDataMutex;
    std::unordered_map<std::string, ZanServer> serverData;
    mutable pthread_mutex_t queryStatsMutex;
    QueryStats queryStats;

private:
    std::vector<MuninPlugin *> muninPlugins;

    pthread_t muninThread;
    pthread_t zandroThread;
};