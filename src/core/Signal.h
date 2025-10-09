#pragma once

#include <functional>
#include <variant>
#include <optional>
#include <vector>
#include <string>
#include <memory>
#include <algorithm>

#include "../../luau/VM/include/lua.h"
#include "../../luau/VM/include/lualib.h"
#include "../../luau/Compiler/include/luacode.h"

struct Instance;

using SignalArg = std::variant<std::monostate, std::string, bool, double, Instance>;

struct Connection {
    bool Connected = true;
    std::function<void(SignalArg)> Callback;

    void Disconnect() {
        Connected = false;
    }
};

struct Signal {
    lua_State* L = nullptr;
    std::vector<int> LuaConnections;//LUA registry refs
    std::vector<std::function<void(Instance*)>> CppConnections;

    //Lua
    void ConnectLua(lua_State* L, int funcIndex);

    //C++
    void Connect(const std::function<void(Instance*)>& cb);

    //Fire
    void Fire(const std::string& s);
    void Fire(Instance* inst);

    void DisconnectAll();
};
