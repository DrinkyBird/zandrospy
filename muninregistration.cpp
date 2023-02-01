#include <pthread.h>
#include <cstdarg>
#include "muninregistration.h"
#include "main.h"
#include "muninnode.h"

MuninPlugin *MuninPlugin::first = nullptr;

MuninPlugin::MuninPlugin(const std::string &name, ConfigCallback configCallback, FetchCallback fetchCallback) noexcept :
    name(name), configCallback(configCallback), fetchCallback(fetchCallback) {
    if (first != nullptr) {
        next = first;
    }
    first = this;
}
MuninPlugin::~MuninPlugin() = default;

std::string MuninPlugin::getName() const {
    return this->name;
}

void MuninPlugin::config(ExecutionContext &context) {
    this->configCallback(context);
}

void MuninPlugin::fetch(ExecutionContext &context) {
    this->fetchCallback(context);
}


ExecutionContext::ExecutionContext(App *app, MuninNode *node) :
    app(app), node(node) { }

ExecutionContext::~ExecutionContext() = default;

void ExecutionContext::write(const std::string &line) {
    node->send(line);
}

void ExecutionContext::writef(const char *fmt, ...) {
    constexpr size_t bufSize = 512;
    char buffer[bufSize];

    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, bufSize, fmt, args);
    va_end(args);

    write(buffer);
}

const std::unordered_map<std::string, ZanServer> &ExecutionContext::getServerData() const {
    return app->serverData;
}

void ExecutionContext::lockServerData() {
    pthread_mutex_lock(&app->serverDataMutex);
}

void ExecutionContext::unlockServerData() {
    pthread_mutex_unlock(&app->serverDataMutex);
}

MuninPlugin *MuninPlugin::findByName(const std::string &name) {
    for (MuninPlugin *plugin = first; plugin != nullptr; plugin = plugin->next) {
        if (plugin->getName() == name) {
            return plugin;
        }
    }

    return nullptr;
}