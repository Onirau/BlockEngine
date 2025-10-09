#include "DataModel.h"
#include "Workspace.h"
#include "../core/LuaBindings.h"

DataModel* DataModel::Instance = nullptr;

DataModel::DataModel() : ::Instance("DataModel") {
    Name = "game";
    Instance = this;
    InitializeServices();
}

DataModel::~DataModel() {
    if (Instance == this) {
        Instance = nullptr;
    }
}

DataModel* DataModel::GetInstance() {
    if (!Instance) {
        Instance = new DataModel();
    }
    return Instance;
}

::Instance* DataModel::GetService(const std::string& serviceName) {
    auto it = Services.find(serviceName);
    if (it != Services.end()) {
        return it->second;
    }

    //Create service if it doesn't exist
    ::Instance* service = nullptr;

    if (serviceName == "Workspace") {
        service = new Workspace();
    }

    if (service) {
        RegisterService(serviceName, service);
        service->SetParent(this);
    }

    return service;
}

::Instance* DataModel::FindService(const std::string& serviceName) {
    auto it = Services.find(serviceName);
    return (it != Services.end()) ? it->second : nullptr;
}

void DataModel::RegisterService(const std::string& name, ::Instance* service) {
    Services[name] = service;

    //Cache commonly used services
    if (name == "Workspace") {
        WorkspaceService = dynamic_cast<Workspace*>(service);
    }
}

void DataModel::InitializeServices() {
    //Create and register core services
    WorkspaceService = new Workspace();
    RegisterService("Workspace", WorkspaceService);
    WorkspaceService->SetParent(this);
}

//Lua bindings
static int DataModel_GetService(lua_State* L) {
    DataModel** pdm = (DataModel**)luaL_checkudata(L, 1, "DataModelMeta");
    DataModel* dm = *pdm;
    const char* serviceName = luaL_checkstring(L, 2);

    ::Instance* service = dm->GetService(std::string(serviceName));
    if (!service) {
        lua_pushnil(L);
        return 1;
    }

    //Return the appropriate userdata based on service type
    if (strcmp(serviceName, "Workspace") == 0) {
        Workspace** udata = (Workspace**)lua_newuserdata(L, sizeof(Workspace*));
        *udata = dynamic_cast<Workspace*>(service);
        luaL_getmetatable(L, "WorkspaceMeta");
        lua_setmetatable(L, -2);
    } else {
        //Generic instance
        ::Instance** udata = (::Instance**)lua_newuserdata(L, sizeof(::Instance*));
        *udata = service;
        luaL_getmetatable(L, "Instance");
        lua_setmetatable(L, -2);
    }

    return 1;
}

static int DataModel_index(lua_State* L) {
    DataModel** pdm = (DataModel**)luaL_checkudata(L, 1, "DataModelMeta");
    const char* key = luaL_checkstring(L, 2);

    if (strcmp(key, "GetService") == 0) {
        lua_pushcfunction(L, DataModel_GetService, "GetService");
        return 1;
    }

    //Delegate to Instance index
    lua_pushnil(L);
    return 1;
}

void DataModel::Bind(lua_State* L) {
    luaL_newmetatable(L, "DataModelMeta");

    lua_pushcfunction(L, DataModel_index, "__index");
    lua_setfield(L, -2, "__index");

    lua_pop(L, 1);

    //Create global 'game' variable
    DataModel* dm = DataModel::GetInstance();
    DataModel** udata = (DataModel**)lua_newuserdata(L, sizeof(DataModel*));
    *udata = dm;
    luaL_getmetatable(L, "DataModelMeta");
    lua_setmetatable(L, -2);
    lua_setglobal(L, "game");
}