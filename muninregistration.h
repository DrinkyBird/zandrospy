#pragma once
#include <functional>
#include <unordered_map>
#include <string>
#include "zanserver.h"

class App;
class MuninNode;
class MuninPlugin;
class ExecutionContext;

typedef void (*ConfigCallback)(ExecutionContext &);
typedef void (*FetchCallback)(ExecutionContext &);

class MuninPlugin {
public:
    MuninPlugin(const std::string &name, ConfigCallback configCallback, FetchCallback fetchCallback) noexcept;
    ~MuninPlugin();

    [[nodiscard]] std::string getName() const;
    void config(ExecutionContext &context);
    void fetch(ExecutionContext &context);

    static MuninPlugin *first;
    MuninPlugin *next;

    static MuninPlugin *findByName(const std::string &name);

private:
    std::string name;
    ConfigCallback configCallback;
    FetchCallback fetchCallback;
};

class ExecutionContext {
public:
    explicit ExecutionContext(App *app, MuninNode *node);
    ~ExecutionContext();

    void write(const std::string &line);
    void writef(const char *fmt, ...);

    [[nodiscard]] const std::unordered_map<std::string, ZanServer> &getServerData() const;
    void lockServerData();
    void unlockServerData();

private:
    App *app;
    MuninNode *node;
};

#define DEFINE_PLUGIN_CONFIG(name) static void __config_##name(::ExecutionContext &ctx)
#define DEFINE_PLUGIN_FETCH(name) static void __fetch_##name(::ExecutionContext &ctx)
#define REGISTER_PLUGIN(name) \
    static void __config_##name(::ExecutionContext &);    \
    static void __fetch_##name(::ExecutionContext &);     \
    static ::MuninPlugin __register_##name(#name, __config_##name, __fetch_##name)
