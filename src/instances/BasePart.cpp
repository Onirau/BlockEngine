#include "BasePart.h"
#include "../core/PropertyMap.h"
#include "../core/LuaBindings.h"

static bool userdata_is_meta(lua_State* L, int idx, const char* meta) {
    if (!lua_isuserdata(L, idx)) return false;
    if (!lua_getmetatable(L, idx)) return false; // pushes metatable
    luaL_getmetatable(L, meta); // pushes registered metatable
    bool eq = lua_rawequal(L, -1, -2);
    lua_pop(L, 2);
    return eq;
}

static BasePart* get_basepart_from_any(lua_State* L, int idx) {
    if (userdata_is_meta(L, idx, "BasePartMeta")) {
        return *(BasePart**)lua_touserdata(L, idx);
    }
    if (userdata_is_meta(L, idx, "PartMeta")) {
        // Part derives from BasePart; safe to reinterpret pointer
        return *(BasePart**)lua_touserdata(L, idx);
    }
    luaL_typeerrorL(L, idx, "BasePart or Part userdata");
    return nullptr;
}

static int BasePart_tostring(lua_State* L) {
    BasePart* part = get_basepart_from_any(L, 1);

    char buf[256];
    snprintf(buf, sizeof(buf),
             "BasePart(%s, pos=(%.2f, %.2f, %.2f), size=(%.2f, %.2f, %.2f), color=(%.2f, %.2f, %.2f))",
             part->Name.c_str(),
             part->Position.x, part->Position.y, part->Position.z,
             part->Size.x, part->Size.y, part->Size.z,
             part->Color.r, part->Color.g, part->Color.b);

    lua_pushstring(L, buf);
    return 1;
}

static int BasePart_index(lua_State* L) {
    BasePart* part = get_basepart_from_any(L, 1);
    const char* key = luaL_checkstring(L, 2);

    // Property map for common scalar properties
    auto get_Name = [](lua_State* L) -> int {
        BasePart* p = get_basepart_from_any(L, 1);
        lua_pushstring(L, p->Name.c_str());
        return 1;
    };
    auto get_ClassName = [](lua_State* L) -> int {
        BasePart* p = get_basepart_from_any(L, 1);
        lua_pushstring(L, p->ClassName.c_str());
        return 1;
    };
    auto get_Anchored = [](lua_State* L) -> int {
        BasePart* p = get_basepart_from_any(L, 1);
        lua_pushboolean(L, p->Anchored);
        return 1;
    };
    auto get_CanCollide = [](lua_State* L) -> int {
        BasePart* p = get_basepart_from_any(L, 1);
        lua_pushboolean(L, p->CanCollide);
        return 1;
    };
    auto get_Transparency = [](lua_State* L) -> int {
        BasePart* p = get_basepart_from_any(L, 1);
        lua_pushnumber(L, p->Transparency);
        return 1;
    };

    const LuaPropertyEntry props[] = {
        {"Name", get_Name, nullptr},
        {"ClassName", get_ClassName, nullptr},
        {"Anchored", get_Anchored, nullptr},
        {"CanCollide", get_CanCollide, nullptr},
        {"Transparency", get_Transparency, nullptr},
        {nullptr, nullptr, nullptr}
    };

    if (lua_property_index(L, props)) return 1;

    if (strcmp(key, "Position") == 0) {
        Vector3Game* v = (Vector3Game*)lua_newuserdata(L, sizeof(Vector3Game));
        *v = part->Position;
        luaL_getmetatable(L, "Vector3Meta");
        lua_setmetatable(L, -2);
    } else if (strcmp(key, "Rotation") == 0) {
        Vector3Game* v = (Vector3Game*)lua_newuserdata(L, sizeof(Vector3Game));
        *v = part->Rotation;
        luaL_getmetatable(L, "Vector3Meta");
        lua_setmetatable(L, -2);
    } else if (strcmp(key, "Size") == 0) {
        Vector3Game* v = (Vector3Game*)lua_newuserdata(L, sizeof(Vector3Game));
        *v = part->Size;
        luaL_getmetatable(L, "Vector3Meta");
        lua_setmetatable(L, -2);
    } else if (strcmp(key, "Color") == 0) {
        Color3* c = (Color3*)lua_newuserdata(L, sizeof(Color3));
        *c = part->Color;
        luaL_getmetatable(L, "Color3Meta");
        lua_setmetatable(L, -2);
    } else {
        // Fallback to Instance __index for methods/properties not handled here
        luaL_getmetatable(L, "Instance");
        lua_getfield(L, -1, "__index");
        lua_remove(L, -2); // remove metatable, keep __index
        lua_pushvalue(L, 1); // userdata
        lua_pushvalue(L, 2); // key
        lua_call(L, 2, 1);
        return 1;
    }

    return 1;
}

static int BasePart_newindex(lua_State* L) {
    BasePart* part = get_basepart_from_any(L, 1);
    const char* key = luaL_checkstring(L, 2);

    // Property map for common scalar properties
    auto set_Name = [](lua_State* L) -> int {
        BasePart* p = get_basepart_from_any(L, 1);
        p->Name = luaL_checkstring(L, 3);
        return 0;
    };
    auto set_Anchored = [](lua_State* L) -> int {
        BasePart* p = get_basepart_from_any(L, 1);
        p->Anchored = lua_toboolean(L, 3) != 0;
        return 0;
    };
    auto set_Transparency = [](lua_State* L) -> int {
        BasePart* p = get_basepart_from_any(L, 1);
        p->Transparency = (float)luaL_checknumber(L, 3);
        return 0;
    };
    auto set_CanCollide = [](lua_State* L) -> int {
        BasePart* p = get_basepart_from_any(L, 1);
        p->CanCollide = lua_toboolean(L, 3) != 0;
        return 0;
    };

    const LuaPropertyEntry props[] = {
        {"Name", nullptr, set_Name},
        {"Anchored", nullptr, set_Anchored},
        {"Transparency", nullptr, set_Transparency},
        {"CanCollide", nullptr, set_CanCollide},
        {nullptr, nullptr, nullptr}
    };

    if (lua_property_newindex(L, props)) return 0;

    if (strcmp(key, "Position") == 0) {
        Vector3Game* v = (Vector3Game*)luaL_checkudata(L, 3, "Vector3Meta");
        part->Position = *v;
    } else if (strcmp(key, "Rotation") == 0) {
        Vector3Game* v = (Vector3Game*)luaL_checkudata(L, 3, "Vector3Meta");
        part->Rotation = *v;
    } else if (strcmp(key, "Size") == 0) {
        Vector3Game* v = (Vector3Game*)luaL_checkudata(L, 3, "Vector3Meta");
        part->Size = *v;
    } else if (strcmp(key, "Color") == 0) {
        Color3* c = (Color3*)luaL_checkudata(L, 3, "Color3Meta");
        part->Color = *c;
    } else {
        // Fallback to Instance __newindex for properties not handled here
        luaL_getmetatable(L, "Instance");
        lua_getfield(L, -1, "__newindex");
        lua_remove(L, -2); // remove metatable, keep __newindex
        lua_pushvalue(L, 1); // userdata
        lua_pushvalue(L, 2); // key
        lua_pushvalue(L, 3); // value
        lua_call(L, 3, 0);
    }

    return 0;
}

static int BasePart_gc(lua_State* L) {
    return 0;
}

void BasePart_Bind(lua_State* L) {
    luaL_newmetatable(L, "BasePartMeta");
    lua_pushcfunction(L, BasePart_index, "__index");
    lua_setfield(L, -2, "__index");
    lua_pushcfunction(L, BasePart_newindex, "__newindex");
    lua_setfield(L, -2, "__newindex");
    lua_pushcfunction(L, BasePart_tostring, "__tostring");
    lua_setfield(L, -2, "__tostring");
    lua_pushcfunction(L, BasePart_gc, "__gc");
    lua_setfield(L, -2, "__gc");

    // Equality by underlying pointer
    lua_pushcfunction(L, Lua_UserdataPtrEq, "__eq");
    lua_setfield(L, -2, "__eq");

    // Inherit from Instance
    luaL_getmetatable(L, "Instance");
    lua_setmetatable(L, -2);

    lua_pop(L, 1);
}

bool BasePart::IsA(const std::string& className) const {
    return this->ClassName == className || Instance::IsA(className);
}
