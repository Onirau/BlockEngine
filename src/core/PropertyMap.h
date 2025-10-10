#pragma once

#include "../../dependencies/luau/VM/include/lua.h"

struct LuaPropertyEntry {
    const char *name;
    int (*get)(lua_State *L);
    int (*set)(lua_State *L);
};

inline int lua_property_index(lua_State *L, const LuaPropertyEntry *entries) {
    const char *key = luaL_checkstring(L, 2);
    for (const LuaPropertyEntry *e = entries; e && e->name; ++e) {
        if (strcmp(key, e->name) == 0) {
            return e->get ? e->get(L) : 0;
        }
    }
    return 0;
}

inline int lua_property_newindex(lua_State *L,
                                 const LuaPropertyEntry *entries) {
    const char *key = luaL_checkstring(L, 2);
    for (const LuaPropertyEntry *e = entries; e && e->name; ++e) {
        if (strcmp(key, e->name) == 0) {
            return e->set ? e->set(L) : 0;
        }
    }
    return 0;
}
