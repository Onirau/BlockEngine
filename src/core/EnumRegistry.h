#pragma once

#include <functional>
#include <vector>

#include "../../luau/VM/include/lua.h"
#include "../../luau/VM/include/lualib.h"

// Function signature for per-enum registration functions
using EnumRegisterFunc = void (*)(lua_State *L);

// Registers all enums into a global Enum table
void RegisterAllEnums(lua_State *L);

// Registrar helper that modules use to auto-register their enum into the list
struct EnumRegistrar {
    explicit EnumRegistrar(EnumRegisterFunc func);
};

// Enum item userdata helpers
struct LuaEnumItem {
    const char *enumName;
    const char *itemName;
    int value;
};

// Returns 1 if idx is an EnumItem and fills outputs; otherwise 0
int TryGetEnumItem(lua_State *L, int idx, const char **outEnumName,
                   const char **outItemName, int *outValue);

// C-style entries for declaring enums in C++
struct EnumEntry {
    const char *name;
    int value;
};

// Registers a single enum table under Enum.<enumName>
void RegisterEnum(lua_State *L, const char *enumName,
                  const EnumEntry *entries /* null-terminated: name==nullptr */);

// Registers Enum.<enumName> from a contiguous names array; values assigned as base+i
void RegisterEnumByNames(lua_State *L, const char *enumName,
                         const char *const *names, int count, int base = 0);

// Convenience macros to declare enums succinctly (numeric enums)
// Usage:
//   LUA_ENUM_BEGIN(PartType)
//     LUA_ENUM_NUM("Ball", 1)
//     LUA_ENUM_NUM("Block", 2)
//   LUA_ENUM_END(PartType)
#define LUA_ENUM_BEGIN(EnumName)                                               \
    static void RegisterEnum_##EnumName(lua_State *L) {                        \
        static const EnumEntry kEntries_##EnumName[] = {

#define LUA_ENUM_NUM(Key, NumValue)                                            \
            {Key, NumValue},

#define LUA_ENUM_END(EnumName)                                                 \
            {nullptr, 0}};                                                     \
        RegisterEnum(L, #EnumName, kEntries_##EnumName);                       \
    }                                                                          \
    static EnumRegistrar s_registrar_##EnumName(RegisterEnum_##EnumName);


