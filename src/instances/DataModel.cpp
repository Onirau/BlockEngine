#include "DataModel.h"
#include "../core/LuaBindings.h"
#include "../core/LuaClassBinder.h"
#include "Workspace.h"

DataModel *DataModel::Instance = nullptr;

DataModel::DataModel() : ServiceProvider("DataModel") {
    Name = "game";
    Instance = this;
    InitializeServices();
}

DataModel::~DataModel() {
    if (Instance == this) {
        Instance = nullptr;
    }
}

DataModel *DataModel::GetInstance() {
    if (!Instance) {
        Instance = new DataModel();
    }
    return Instance;
}

::Instance *DataModel::CreateService(const std::string &serviceName) {
    ::Instance *service = nullptr;

    if (serviceName == "Workspace") {
        service = new Workspace();
        // Cache commonly used services
        WorkspaceService = dynamic_cast<Workspace *>(service);
    }
    // Add more services here as needed
    // else if (serviceName == "Players") {
    //     service = new Players();
    // }

    return service;
}

void DataModel::InitializeServices() {
    // Create and register core services
    WorkspaceService = new Workspace();
    RegisterService("Workspace", WorkspaceService);
    WorkspaceService->SetParent(this);
}

bool DataModel::IsA(const std::string &className) const {
    return className == "DataModel" || ServiceProvider::IsA(className);
}

void DataModel::Bind(lua_State *L) {
    LuaClassBinder::RegisterClass("DataModel", "ServiceProvider");

    // Create global 'game' variable
    DataModel *dm = DataModel::GetInstance();
    LuaClassBinder::PushInstance(L, dm);
    lua_setglobal(L, "game");
}