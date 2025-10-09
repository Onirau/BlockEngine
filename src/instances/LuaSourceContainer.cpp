#include "LuaSourceContainer.h"
#include "../core/PropertyMap.h"
#include "../core/LuaBindings.h"

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
    if (userdata_is_meta(L, idx, "PartMeta")) {
        //Part derives from LuaSourceContainer; safe to reinterpret pointer
        return *(LuaSourceContainer**)lua_touserdata(L, idx);
    }
    luaL_typeerrorL(L, idx, "LuaSourceContainer or Part userdata");
    return nullptr;
}

static int LuaSourceContainer_tostring(lua_State* L) {
    LuaSourceContainer* part = get_luasourcecontainer_from_any(L, 1);

    char buf[256];
    snprintf(buf, sizeof(buf),
             "LuaSourceContainer(%s)",
             part->Name.c_str());

    lua_pushstring(L, buf);
    return 1;
}

static int LuaSourceContainer_index(lua_State* L) {
    LuaSourceContainer* part = get_luasourcecontainer_from_any(L, 1);
    const char* key = luaL_checkstring(L, 2);

    //Property map for common scalar properties
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

    const LuaPropertyEntry props[] = {
        {"Name", get_Name, nullptr},
        {"ClassName", get_ClassName, nullptr},
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

    return 1;
}

static int LuaSourceContainer_newindex(lua_State* L) {
    LuaSourceContainer* part = get_luasourcecontainer_from_any(L, 1);
    const char* key = luaL_checkstring(L, 2);

    //Property map for common scalar properties
    auto set_Name = [](lua_State* L) -> int {
        LuaSourceContainer* p = get_luasourcecontainer_from_any(L, 1);
        p->Name = luaL_checkstring(L, 3);
        return 0;
    };

    const LuaPropertyEntry props[] = {
        {"Name", nullptr, set_Name},
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
