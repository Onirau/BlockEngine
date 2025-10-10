#include "LuaSourceContainer.h"
#include "../core/LuaBindings.h"
#include "../core/LuaClassBinder.h"
#include "../datatypes/Task.h"
#include <fstream>
#include <sstream>

LuaSourceContainer::LuaSourceContainer(const std::string &className)
    : Instance(className) {
    ClassName = className;
    Name = className;
}

bool LuaSourceContainer::LoadFromPath() {
    if (SourcePath.empty()) {
        return false;
    }

    std::ifstream file(SourcePath);
    if (!file.is_open()) {
        printf("Failed to open script file: %s\n", SourcePath.c_str());
        return false;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    Source = buffer.str();
    file.close();

    return true;
}

bool LuaSourceContainer::Execute(lua_State *L) {
    if (!Enabled) {
        return false;
    }

    std::string scriptSource = Source;

    // If Source is empty but SourcePath is set, load from file
    if (scriptSource.empty() && !SourcePath.empty()) {
        if (!LoadFromPath()) {
            return false;
        }
        scriptSource = Source;
    }

    // If still empty, nothing to execute
    if (scriptSource.empty()) {
        return false;
    }

    // Execute using the Task system
    return Task_RunScript(L, scriptSource);
}

bool LuaSourceContainer::IsA(const std::string &className) const {
    return this->ClassName == className || Instance::IsA(className);
}

void LuaSourceContainer_Bind(lua_State *L) {
    LuaClassBinder::RegisterClass("LuaSourceContainer", "Instance");

    // Enabled property
    LuaClassBinder::AddProperty(
        "LuaSourceContainer", "Enabled",
        [](lua_State *L, Instance *inst) -> int {
            auto *container = static_cast<LuaSourceContainer *>(inst);
            lua_pushboolean(L, container->Enabled);
            return 1;
        },
        [](lua_State *L, Instance *inst, int valueIdx) -> int {
            auto *container = static_cast<LuaSourceContainer *>(inst);
            container->Enabled = lua_toboolean(L, valueIdx) != 0;
            return 0;
        });

    // Source property
    LuaClassBinder::AddProperty(
        "LuaSourceContainer", "Source",
        [](lua_State *L, Instance *inst) -> int {
            auto *container = static_cast<LuaSourceContainer *>(inst);
            lua_pushstring(L, container->Source.c_str());
            return 1;
        },
        [](lua_State *L, Instance *inst, int valueIdx) -> int {
            auto *container = static_cast<LuaSourceContainer *>(inst);
            container->Source = luaL_checkstring(L, valueIdx);
            return 0;
        });

    // SourcePath property
    LuaClassBinder::AddProperty(
        "LuaSourceContainer", "SourcePath",
        [](lua_State *L, Instance *inst) -> int {
            auto *container = static_cast<LuaSourceContainer *>(inst);
            lua_pushstring(L, container->SourcePath.c_str());
            return 1;
        },
        [](lua_State *L, Instance *inst, int valueIdx) -> int {
            auto *container = static_cast<LuaSourceContainer *>(inst);
            container->SourcePath = luaL_checkstring(L, valueIdx);
            return 0;
        });

    // Execute method
    LuaClassBinder::AddMethod("LuaSourceContainer", "Execute",
                              [](lua_State *L, Instance *inst) -> int {
                                  auto *container =
                                      static_cast<LuaSourceContainer *>(inst);
                                  bool success = container->Execute(L);
                                  lua_pushboolean(L, success);
                                  return 1;
                              });

    // LoadFromPath method
    LuaClassBinder::AddMethod("LuaSourceContainer", "LoadFromPath",
                              [](lua_State *L, Instance *inst) -> int {
                                  auto *container =
                                      static_cast<LuaSourceContainer *>(inst);
                                  bool success = container->LoadFromPath();
                                  lua_pushboolean(L, success);
                                  return 1;
                              });
}