#include "DataModel.h"
#include "Workspace.h"
#include "../core/LuaBindings.h"
#include "../core/LuaClassBinder.h"

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

void DataModel::Bind(lua_State* L) {
    LuaClassBinder::RegisterClass("DataModel", "Instance");

    //GetService method
    LuaClassBinder::AddMethod("DataModel", "GetService",
                              [](lua_State* L, ::Instance* inst) -> int {
                                  auto* dm = static_cast<DataModel*>(inst);
                                  const char* serviceName = luaL_checkstring(L, 2);

                                  ::Instance* service = dm->GetService(std::string(serviceName));
                                  if (service) {
                                      LuaClassBinder::PushInstance(L, service);
                                  } else {
                                      lua_pushnil(L);
                                  }
                                  return 1;
                              });

    //FindService method
    LuaClassBinder::AddMethod("DataModel", "FindService",
                              [](lua_State* L, ::Instance* inst) -> int {
                                  auto* dm = static_cast<DataModel*>(inst);
                                  const char* serviceName = luaL_checkstring(L, 2);

                                  ::Instance* service = dm->FindService(std::string(serviceName));
                                  if (service) {
                                      LuaClassBinder::PushInstance(L, service);
                                  } else {
                                      lua_pushnil(L);
                                  }
                                  return 1;
                              });

    //Create global 'game' variable
    DataModel* dm = DataModel::GetInstance();
    LuaClassBinder::PushInstance(L, dm);
    lua_setglobal(L, "game");
}