#include "EnumRegistry.h"

static const char *kEnumItemMeta = "EnumItem";
static const char *kReadOnlyMeta = "ReadOnlyTable";

namespace {
    std::vector<EnumRegisterFunc> &getRegistry() {
        static std::vector<EnumRegisterFunc> funcs;
        return funcs;
    }
}

// Read-only table support
static int ReadOnlyNewIndex(lua_State *L) {
    const char *key = lua_tostring(L, 2);
    luaL_error(L, "attempt to modify read-only table%s%s", key ? ": " : "", key ? key : "");
    return 0;
}

static void EnsureReadOnlyMetatable(lua_State *L) {
    if (luaL_newmetatable(L, kReadOnlyMeta)) {
        lua_pushcfunction(L, ReadOnlyNewIndex, "__newindex");
        lua_setfield(L, -2, "__newindex");
        lua_pushstring(L, "locked");
        lua_setfield(L, -2, "__metatable");
    }
    lua_pop(L, 1);
}

static void MakeTableReadOnly(lua_State *L, int idx) {
    EnsureReadOnlyMetatable(L);
    if (idx < 0) idx = lua_gettop(L) + 1 + idx;
    luaL_getmetatable(L, kReadOnlyMeta);
    lua_setmetatable(L, idx);
}

// Proxy-based read-only that intercepts writes to existing keys
static int RO_Index(lua_State *L) {
    // upvalue1 = target table
    lua_pushvalue(L, lua_upvalueindex(1)); // target
    lua_pushvalue(L, 2);                   // key
    lua_gettable(L, -2);                   // target[key]
    lua_replace(L, -2);                    // pop target, leave value
    return 1;
}

static int RO_NewIndex(lua_State *L) {
    const char *key = lua_tostring(L, 2);
    luaL_error(L, "attempt to modify read-only table%s%s", key ? ": " : "", key ? key : "");
    return 0;
}

static void MakeReadOnlyProxy(lua_State *L, int idx) {
    if (idx < 0) idx = lua_gettop(L) + 1 + idx;

    // Stack: ... original(idx)
    lua_pushvalue(L, idx);                 // upvalue: target
    // Create proxy table
    lua_newtable(L);                       // proxy
    // Metatable for proxy
    lua_newtable(L);                       // meta (fresh per proxy)
    // __index closure with upvalue = target
    lua_pushvalue(L, -3);                  // push target as upvalue
    lua_pushcclosure(L, RO_Index, "__index", 1);
    lua_setfield(L, -2, "__index");
    // __newindex closure with upvalue = target (unused but keeps symmetry)
    lua_pushvalue(L, -3);
    lua_pushcclosure(L, RO_NewIndex, "__newindex", 1);
    lua_setfield(L, -2, "__newindex");
    // lock metatable
    lua_pushstring(L, "locked");
    lua_setfield(L, -2, "__metatable");
    // set metatable on proxy
    lua_setmetatable(L, -2);               // set meta for proxy

    // Replace original at idx with proxy
    lua_replace(L, idx);                   // idx now proxy; target remains at top-1
    // Pop remaining target
    lua_pop(L, 1);
}

// EnumItem methods
static int EnumItem_tostring(lua_State *L) {
    LuaEnumItem *item = (LuaEnumItem *)luaL_checkudata(L, 1, kEnumItemMeta);
    lua_pushfstring(L, "Enum.%s.%s", item->enumName, item->itemName);
    return 1;
}

static int EnumItem_index(lua_State *L) {
    LuaEnumItem *item = (LuaEnumItem *)luaL_checkudata(L, 1, kEnumItemMeta);
    const char *key = luaL_checkstring(L, 2);
    if (strcmp(key, "Value") == 0) {
        lua_pushinteger(L, item->value);
        return 1;
    }
    if (strcmp(key, "Name") == 0) {
        lua_pushstring(L, item->itemName);
        return 1;
    }
    lua_pushnil(L);
    return 1;
}

static void EnsureEnumItemMetatable(lua_State *L) {
    if (luaL_newmetatable(L, kEnumItemMeta)) {
        lua_pushcfunction(L, EnumItem_index, "__index");
        lua_setfield(L, -2, "__index");
        lua_pushcfunction(L, EnumItem_tostring, "__tostring");
        lua_setfield(L, -2, "__tostring");
    }
    lua_pop(L, 1);
}

int TryGetEnumItem(lua_State *L, int idx, const char **outEnumName,
                   const char **outItemName, int *outValue) {
    if (!lua_isuserdata(L, idx)) return 0;
    // luaL_testudata isn't available; use lua_getmetatable + compare
    LuaEnumItem *p = (LuaEnumItem *)luaL_checkudata(L, idx, kEnumItemMeta);
    if (!p) return 0;
    if (outEnumName) *outEnumName = p->enumName;
    if (outItemName) *outItemName = p->itemName;
    if (outValue) *outValue = p->value;
    return 1;
}

static void PushEnumItem(lua_State *L, const char *enumName,
                         const char *itemName, int value) {
    EnsureEnumItemMetatable(L);
    LuaEnumItem *ud = (LuaEnumItem *)lua_newuserdata(L, sizeof(LuaEnumItem));
    ud->enumName = enumName;
    ud->itemName = itemName;
    ud->value = value;
    luaL_getmetatable(L, kEnumItemMeta);
    lua_setmetatable(L, -2);
}

EnumRegistrar::EnumRegistrar(EnumRegisterFunc func) {
    getRegistry().push_back(func);
}

void RegisterEnum(lua_State *L, const char *enumName, const EnumEntry *entries) {
    // Ensure global Enum table exists
    lua_getglobal(L, "Enum");
    if (lua_isnil(L, -1)) {
        lua_pop(L, 1);
        lua_newtable(L);
        lua_setglobal(L, "Enum");
        lua_getglobal(L, "Enum");
    }

    // Create specific Enum.<enumName> real table
    lua_newtable(L);                       // real

    for (const EnumEntry *e = entries; e && e->name; ++e) {
        PushEnumItem(L, enumName, e->name, e->value);
        lua_setfield(L, -2, e->name);
    }

    // Wrap in read-only proxy before attaching
    MakeReadOnlyProxy(L, -1);

    // Set Enum[enumName] = table, then pop Enum
    lua_setfield(L, -2, enumName);
    lua_pop(L, 1);
}

void RegisterEnumByNames(lua_State *L, const char *enumName,
                         const char *const *names, int count, int base) {
    lua_getglobal(L, "Enum");
    if (lua_isnil(L, -1)) {
        lua_pop(L, 1);
        lua_newtable(L);
        lua_setglobal(L, "Enum");
        lua_getglobal(L, "Enum");
    }

    lua_newtable(L);                       // real
    for (int i = 0; i < count; ++i) {
        if (!names[i]) continue;
        PushEnumItem(L, enumName, names[i], base + i);
        lua_setfield(L, -2, names[i]);
    }
    // Wrap in read-only proxy before attaching
    MakeReadOnlyProxy(L, -1);
    lua_setfield(L, -2, enumName);
    lua_pop(L, 1);
}

void RegisterAllEnums(lua_State *L) {
    // Build into a fresh real Enum table, then expose a read-only proxy globally
    lua_newtable(L);                       // real Enum
    lua_setglobal(L, "Enum");

    for (auto func : getRegistry()) {
        func(L);
    }

    // Get real Enum and wrap in proxy
    lua_getglobal(L, "Enum");            // real
    MakeReadOnlyProxy(L, -1);             // convert to proxy on stack
    lua_setglobal(L, "Enum");            // set proxy as global
}


