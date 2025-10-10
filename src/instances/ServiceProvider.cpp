#include "ServiceProvider.h"
#include "../core/LuaBindings.h"
#include "../core/LuaClassBinder.h"

ServiceProvider::ServiceProvider(const std::string &className)
    : Instance(className) {}

ServiceProvider::~ServiceProvider() {
    // Services are cleaned up through the instance hierarchy
    Services.clear();
}

::Instance *ServiceProvider::GetService(const std::string &serviceName) {
    auto it = Services.find(serviceName);
    if (it != Services.end()) {
        return it->second;
    }

    // Create service if it doesn't exist (delegate to derived class)
    ::Instance *service = CreateService(serviceName);

    if (service) {
        RegisterService(serviceName, service);
        service->SetParent(this);
    }

    return service;
}

::Instance *ServiceProvider::FindService(const std::string &serviceName) {
    auto it = Services.find(serviceName);
    return (it != Services.end()) ? it->second : nullptr;
}

void ServiceProvider::RegisterService(const std::string &name,
                                      ::Instance *service) {
    Services[name] = service;
}

bool ServiceProvider::IsA(const std::string &className) const {
    return className == "ServiceProvider" || Instance::IsA(className);
}

void ServiceProvider::Bind(lua_State *L) {
    LuaClassBinder::RegisterClass("ServiceProvider", "Instance");

    // GetService method
    LuaClassBinder::AddMethod(
        "ServiceProvider", "GetService",
        [](lua_State *L, ::Instance *inst) -> int {
            auto *sp = static_cast<ServiceProvider *>(inst);
            const char *serviceName = luaL_checkstring(L, 2);

            ::Instance *service = sp->GetService(std::string(serviceName));
            if (service) {
                LuaClassBinder::PushInstance(L, service);
            } else {
                lua_pushnil(L);
            }
            return 1;
        });

    // FindService method
    LuaClassBinder::AddMethod(
        "ServiceProvider", "FindService",
        [](lua_State *L, ::Instance *inst) -> int {
            auto *sp = static_cast<ServiceProvider *>(inst);
            const char *serviceName = luaL_checkstring(L, 2);

            ::Instance *service = sp->FindService(std::string(serviceName));
            if (service) {
                LuaClassBinder::PushInstance(L, service);
            } else {
                lua_pushnil(L);
            }
            return 1;
        });
}