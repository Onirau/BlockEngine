#include "DataModel.h"
#include "../core/LuaBindings.h"
#include "../core/LuaClassBinder.h"
#include "services/Lighting.h"
#include "services/Workspace.h"

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

    if (serviceName == "Lighting") {
        service = new Lighting();
        // Cache commonly used services
        LightingService = dynamic_cast<Lighting *>(service);
    }
    // Add more services here as needed
    // else if (serviceName == "Players") {
    //     service = new Players();
    // }

    return service;
}

void DataModel::InitializeServices() {
    // Register Services
    WorkspaceService = new Workspace();
    RegisterService("Workspace", WorkspaceService);
    WorkspaceService->SetParent(this);

    LightingService = new Lighting();
    RegisterService("Lighting", LightingService);
    LightingService->SetParent(this);
}

bool DataModel::IsA(const std::string &className) const {
    return className == "DataModel" || ServiceProvider::IsA(className);
}

void DataModel::Bind(lua_State *L) {
    LuaClassBinder::RegisterClass("DataModel", "ServiceProvider");
}