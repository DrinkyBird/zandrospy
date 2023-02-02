#include <pthread.h>
#include "muninregistration.h"
#include "main.h"
#include "muninnode.h"

MuninPlugin *MuninPlugin::first = nullptr;

MuninPlugin::MuninPlugin(const std::string &name, PluginCallback callback) noexcept :
    name(name), callback(callback) {
    if (first != nullptr) {
        next = first;
    }
    first = this;
}
MuninPlugin::~MuninPlugin() = default;

std::string MuninPlugin::getName() const {
    return this->name;
}

void MuninPlugin::execute(ExecutionContext &context) {
    this->callback(context);
}


ExecutionContext::ExecutionContext(App *app, MuninNode *node, bool config, bool fetch) :
    app(app), node(node), fetch(fetch), config(config) { }

ExecutionContext::~ExecutionContext() = default;

void ExecutionContext::write(const std::string &line) {
    node->send(line);
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