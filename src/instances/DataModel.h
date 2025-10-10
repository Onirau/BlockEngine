#pragma once

#include "Instance.h"
#include <memory>
#include <unordered_map>

#include "../../luau/Compiler/include/luacode.h"
#include "../../luau/VM/include/lua.h"
#include "../../luau/VM/include/lualib.h"

struct Workspace;

struct DataModel : public Instance {
    static DataModel *Instance;

    std::unordered_map<std::string, ::Instance *> Services;

    Workspace *WorkspaceService = nullptr;

    DataModel();
    virtual ~DataModel();

    static DataModel *GetInstance();

    ::Instance *GetService(const std::string &serviceName);
    ::Instance *FindService(const std::string &serviceName);

    void RegisterService(const std::string &name, ::Instance *service);

    void InitializeServices();

    static void Bind(lua_State *L);
};