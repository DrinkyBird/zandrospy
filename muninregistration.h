#pragma once
#include <functional>
#include <unordered_map>
#include <string>
#include "zanserver.h"

class App;
class MuninNode;
class MuninPlugin;
class ExecutionContext;

typedef void (*PluginCallback)(ExecutionContext &);

class MuninPlugin {
public:
    MuninPlugin(const std::string &name, PluginCallback callback) noexcept;
    ~MuninPlugin();

    [[nodiscard]] std::string getName() const;
    void execute(ExecutionContext &context);

    static MuninPlugin *first;
    MuninPlugin *next;

    static MuninPlugin *findByName(const std::string &name);

private:
    std::string name;
    PluginCallback callback;
};

class ExecutionContext {
public:
    explicit ExecutionContext(App *app, MuninNode *node, bool config, bool fetch);
    ~ExecutionContext();

    void write(const std::string &line);
    void writef(const char *fmt, ...);

    [[nodiscard]] const std::unordered_map<std::string, ZanServer> &getServerData() const;
    void lockServerData();
    void unlockServerData();

    inline bool isConfig() const { return config; }
    inline bool isFetch() const { return fetch; }

private:
    App *app;
    MuninNode *node;
    bool config, fetch;
};

#define REGISTER_PLUGIN(name)                                           \
    static void __callback_##name(::ExecutionContext &);                \
    static ::MuninPlugin __register_##name(#name, __callback_##name);   \
    void __callback_##name(::ExecutionContext &ctx)
