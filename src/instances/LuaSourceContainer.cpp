#include "LuaSourceContainer.h"
#include "../core/PropertyMap.h"
#include "../core/LuaBindings.h"
#include "../datatypes/Task.h"
#include <fstream>
#include <sstream>

LuaSourceContainer::LuaSourceContainer(const std::string& className)
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

bool LuaSourceContainer::Execute(lua_State* L) {
    if (!Enabled) {
        return false;
    }

    std::string scriptSource = Source;

    //If Source is empty but SourcePath is set, load from file
    if (scriptSource.empty() && !SourcePath.empty()) {
        if (!LoadFromPath()) {
            return false;
        }
        scriptSource = Source;
    }

    //If still empty, nothing to execute
    if (scriptSource.empty()) {
        return false;
    }

    //Execute using the Task system
    return Task_RunScript(L, scriptSource);
}

static bool userdata_is_meta(lua_State* L, int idx, const char* meta) {
    if (!lua_isuserdata(L, idx)) return false;
    if (!lua_getmetatable(L, idx)) return false;//pushes metatable
    luaL_getmetatable(L, meta);//pushes registered metatable
    bool eq = lua_rawequal(L, -1, -2);
    lua_pop(L, 2);
    return eq;
}

static LuaSourceContainer* get_luasourcecontainer_from_any(lua_State* L, int idx) {
    if (userdata_is_meta(L, idx, "LuaSourceContainerMeta")) {
        return *(LuaSourceContainer**)lua_touserdata(L, idx);
    }
    luaL_typeerrorL(L, idx, "LuaSourceContainer userdata");
    return nullptr;
}

static int LuaSourceContainer_tostring(lua_State* L) {
    LuaSourceContainer* container = get_luasourcecontainer_from_any(L, 1);

    char buf[256];
    snprintf(buf, sizeof(buf),
             "LuaSourceContainer(%s, Enabled=%s, HasSource=%s)",
             container->Name.c_str(),
             container->Enabled ? "true" : "false",
             container->Source.empty() ? "false" : "true");

    lua_pushstring(L, buf);
    return 1;
}

static int LuaSourceContainer_index(lua_State* L) {
    LuaSourceContainer* container = get_luasourcecontainer_from_any(L, 1);
    const char* key = luaL_checkstring(L, 2);

    //Property map for scalar properties
    auto get_Name = [](lua_State* L) -> int {
        LuaSourceContainer* p = get_luasourcecontainer_from_any(L, 1);
        lua_pushstring(L, p->Name.c_str());
        return 1;
    };
    auto get_ClassName = [](lua_State* L) -> int {
        LuaSourceContainer* p = get_luasourcecontainer_from_any(L, 1);
        lua_pushstring(L, p->ClassName.c_str());
        return 1;
    };
    auto get_Enabled = [](lua_State* L) -> int {
        LuaSourceContainer* p = get_luasourcecontainer_from_any(L, 1);
        lua_pushboolean(L, p->Enabled);
        return 1;
    };
    auto get_Source = [](lua_State* L) -> int {
        LuaSourceContainer* p = get_luasourcecontainer_from_any(L, 1);
        lua_pushstring(L, p->Source.c_str());
        return 1;
    };
    auto get_SourcePath = [](lua_State* L) -> int {
        LuaSourceContainer* p = get_luasourcecontainer_from_any(L, 1);
        lua_pushstring(L, p->SourcePath.c_str());
        return 1;
    };

    const LuaPropertyEntry props[] = {
        {"Name", get_Name, nullptr},
        {"ClassName", get_ClassName, nullptr},
        {"Enabled", get_Enabled, nullptr},
        {"Source", get_Source, nullptr},
        {"SourcePath", get_SourcePath, nullptr},
        {nullptr, nullptr, nullptr}};

    if (lua_property_index(L, props)) return 1;

    //Fallback to Instance __index for methods/properties not handled here
    luaL_getmetatable(L, "Instance");
    lua_getfield(L, -1, "__index");
    lua_remove(L, -2);//remove metatable, keep __index
    lua_pushvalue(L, 1);//userdata
    lua_pushvalue(L, 2);//key
    lua_call(L, 2, 1);
    return 1;
}

static int LuaSourceContainer_newindex(lua_State* L) {
    LuaSourceContainer* container = get_luasourcecontainer_from_any(L, 1);
    const char* key = luaL_checkstring(L, 2);

    //Property map for scalar properties
    auto set_Name = [](lua_State* L) -> int {
        LuaSourceContainer* p = get_luasourcecontainer_from_any(L, 1);
        p->Name = luaL_checkstring(L, 3);
        return 0;
    };
    auto set_Enabled = [](lua_State* L) -> int {
        LuaSourceContainer* p = get_luasourcecontainer_from_any(L, 1);
        p->Enabled = lua_toboolean(L, 3) != 0;
        return 0;
    };
    auto set_Source = [](lua_State* L) -> int {
        LuaSourceContainer* p = get_luasourcecontainer_from_any(L, 1);
        p->Source = luaL_checkstring(L, 3);
        return 0;
    };
    auto set_SourcePath = [](lua_State* L) -> int {
        LuaSourceContainer* p = get_luasourcecontainer_from_any(L, 1);
        p->SourcePath = luaL_checkstring(L, 3);
        return 0;
    };

    const LuaPropertyEntry props[] = {
        {"Name", nullptr, set_Name},
        {"Enabled", nullptr, set_Enabled},
        {"Source", nullptr, set_Source},
        {"SourcePath", nullptr, set_SourcePath},
        {nullptr, nullptr, nullptr}};

    if (lua_property_newindex(L, props)) return 0;

    //Fallback to Instance __newindex for properties not handled here
    luaL_getmetatable(L, "Instance");
    lua_getfield(L, -1, "__newindex");
    lua_remove(L, -2);//remove metatable, keep __newindex
    lua_pushvalue(L, 1);//userdata
    lua_pushvalue(L, 2);//key
    lua_pushvalue(L, 3);//value
    lua_call(L, 3, 0);

    return 0;
}

static int LuaSourceContainer_Execute(lua_State* L) {
    LuaSourceContainer* container = get_luasourcecontainer_from_any(L, 1);
    bool success = container->Execute(L);
    lua_pushboolean(L, success);
    return 1;
}

static int LuaSourceContainer_LoadFromPath(lua_State* L) {
    LuaSourceContainer* container = get_luasourcecontainer_from_any(L, 1);
    bool success = container->LoadFromPath();
    lua_pushboolean(L, success);
    return 1;
}

static int LuaSourceContainer_gc(lua_State* L) {
    return 0;
}

void LuaSourceContainer_Bind(lua_State* L) {
    luaL_newmetatable(L, "LuaSourceContainerMeta");

    lua_pushcfunction(L, LuaSourceContainer_index, "__index");
    lua_setfield(L, -2, "__index");
    lua_pushcfunction(L, LuaSourceContainer_newindex, "__newindex");
    lua_setfield(L, -2, "__newindex");
    lua_pushcfunction(L, LuaSourceContainer_tostring, "__tostring");
    lua_setfield(L, -2, "__tostring");
    lua_pushcfunction(L, LuaSourceContainer_gc, "__gc");
    lua_setfield(L, -2, "__gc");

    //Add methods
    lua_pushcfunction(L, LuaSourceContainer_Execute, "Execute");
    lua_setfield(L, -2, "Execute");
    lua_pushcfunction(L, LuaSourceContainer_LoadFromPath, "LoadFromPath");
    lua_setfield(L, -2, "LoadFromPath");

    //Equality by underlying pointer
    lua_pushcfunction(L, Lua_UserdataPtrEq, "__eq");
    lua_setfield(L, -2, "__eq");

    //Inherit from Instance
    luaL_getmetatable(L, "Instance");
    lua_setmetatable(L, -2);

    lua_pop(L, 1);
}

bool LuaSourceContainer::IsA(const std::string& className) const {
    return this->ClassName == className || Instance::IsA(className);
}