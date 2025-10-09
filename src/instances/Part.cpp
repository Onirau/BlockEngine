#include "Part.h"
#include "DataModel.h"
#include <algorithm>

#include "../datatypes/Instance.h"
#include "Workspace.h"
#include "../core/PropertyMap.h"
#include "../core/LuaBindings.h"

const char* validShapes[] = {"Block", "Sphere", "Cylinder", "Wedge", "CornerWedge", nullptr};

Part::Part()
    : BasePart() {
    ClassName = "Part";
    Shape = "Block";
}

Part::Part(const std::string& name,
           const Vector3Game& position,
           const Vector3Game& size,
           const Color3& color,
           bool anchored,
           std::string shape)
    : BasePart() {
    Name = name;
    Position = position;
    Size = size;
    Color = color;
    Anchored = anchored;
    Shape = shape;
    ClassName = "Part";
}


static int Part_index(lua_State* L) {
    Part** ppart = (Part**)luaL_checkudata(L, 1, "PartMeta");
    Part* part = *ppart;
    const char* key = luaL_checkstring(L, 2);

    if (!part) {
        luaL_error(L, "Part pointer is null or deleted");
    }

    //Handle Parent property
    if (strcmp(key, "Parent") == 0) {
        if (part->Parent) {
            if (part->Parent->ClassName == "Workspace") {
                lua_getglobal(L, "workspace");
            } else {
                Part** udata = (Part**)lua_newuserdata(L, sizeof(Part*));
                *udata = dynamic_cast<Part*>(part->Parent);
                luaL_getmetatable(L, "PartMeta");
                lua_setmetatable(L, -2);
            }
        } else {
            lua_pushnil(L);
        }
        return 1;
    }

    // Property map for simple properties
    auto get_Shape = [](lua_State* L) -> int {
        Part** p = (Part**)luaL_checkudata(L, 1, "PartMeta");
        lua_pushstring(L, (*p)->Shape.c_str());
        return 1;
    };
    auto get_Name = [](lua_State* L) -> int {
        Part** p = (Part**)luaL_checkudata(L, 1, "PartMeta");
        lua_pushstring(L, (*p)->Name.c_str());
        return 1;
    };
    auto get_ClassName = [](lua_State* L) -> int {
        Part** p = (Part**)luaL_checkudata(L, 1, "PartMeta");
        lua_pushstring(L, (*p)->ClassName.c_str());
        return 1;
    };
    auto get_Anchored = [](lua_State* L) -> int {
        Part** p = (Part**)luaL_checkudata(L, 1, "PartMeta");
        lua_pushboolean(L, (*p)->Anchored);
        return 1;
    };
    auto get_CanCollide = [](lua_State* L) -> int {
        Part** p = (Part**)luaL_checkudata(L, 1, "PartMeta");
        lua_pushboolean(L, (*p)->CanCollide);
        return 1;
    };
    auto get_Transparency = [](lua_State* L) -> int {
        Part** p = (Part**)luaL_checkudata(L, 1, "PartMeta");
        lua_pushnumber(L, (*p)->Transparency);
        return 1;
    };

    const LuaPropertyEntry props[] = {
        {"Shape", get_Shape, nullptr},
        {"Name", get_Name, nullptr},
        {"ClassName", get_ClassName, nullptr},
        {"Anchored", get_Anchored, nullptr},
        {"CanCollide", get_CanCollide, nullptr},
        {"Transparency", get_Transparency, nullptr},
        {nullptr, nullptr, nullptr}
    };

    if (lua_property_index(L, props)) return 1;

    else if (strcmp(key, "Position") == 0) {
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
        // Fallback to BasePart __index for methods/properties not handled here
        luaL_getmetatable(L, "BasePartMeta");
        lua_getfield(L, -1, "__index");
        lua_remove(L, -2); // remove metatable, keep __index
        lua_pushvalue(L, 1); // userdata
        lua_pushvalue(L, 2); // key
        lua_call(L, 2, 1);
        return 1;
    }

    return 1;
}

static int Part_newindex(lua_State* L) {
    Part** ppart = (Part**)luaL_checkudata(L, 1, "PartMeta");
    Part* part = *ppart;
    const char* key = luaL_checkstring(L, 2);

    if (!part) {
        luaL_error(L, "Part pointer is null");
    }

    //Handle Parent property
    if (strcmp(key, "Parent") == 0) {
        if (lua_isnil(L, 3)) {
            part->SetParent(nullptr);
        } else {
            Instance* newParent = nullptr;

            //Check different parent types. luaL_testudata may not be available
            //in this Luau build, so compare metatables manually.
            auto userdata_is_meta = [&](lua_State* L, int idx, const char* meta) -> bool {
                if (!lua_isuserdata(L, idx)) return false;
                if (!lua_getmetatable(L, idx)) return false;//pushes metatable
                luaL_getmetatable(L, meta);//pushes registered metatable
                bool eq = lua_rawequal(L, -1, -2);
                lua_pop(L, 2);
                return eq;
            };

            if (userdata_is_meta(L, 3, "WorkspaceMeta")) {
                Workspace** pws = (Workspace**)lua_touserdata(L, 3);
                newParent = *pws;
            } else if (userdata_is_meta(L, 3, "PartMeta")) {
                Part** pparent = (Part**)lua_touserdata(L, 3);
                newParent = *pparent;
            }

            if (newParent) {
                part->SetParent(newParent);
            }
        }
        return 0;
    }

    // Property map for simple properties
    auto set_Shape = [](lua_State* L) -> int {
        Part** p = (Part**)luaL_checkudata(L, 1, "PartMeta");
        const char* newShape = luaL_checkstring(L, 3);
        for (auto shape : validShapes) {
            if (shape && strcmp(newShape, shape) == 0) {
                (*p)->Shape = std::string(newShape);
                return 0;
            }
        }
        luaL_error(L, "attempt to set invalid Part.Shape value of '%s'", newShape);
        return 0;
    };
    auto set_Name = [](lua_State* L) -> int {
        Part** p = (Part**)luaL_checkudata(L, 1, "PartMeta");
        (*p)->Name = std::string(luaL_checkstring(L, 3));
        return 0;
    };
    auto set_Anchored = [](lua_State* L) -> int {
        Part** p = (Part**)luaL_checkudata(L, 1, "PartMeta");
        (*p)->Anchored = lua_toboolean(L, 3) != 0;
        return 0;
    };
    auto set_Transparency = [](lua_State* L) -> int {
        Part** p = (Part**)luaL_checkudata(L, 1, "PartMeta");
        (*p)->Transparency = (float)luaL_checknumber(L, 3);
        return 0;
    };

    const LuaPropertyEntry props[] = {
        {"Shape", nullptr, set_Shape},
        {"Name", nullptr, set_Name},
        {"Anchored", nullptr, set_Anchored},
        {"Transparency", nullptr, set_Transparency},
        {nullptr, nullptr, nullptr}
    };

    if (lua_property_newindex(L, props)) return 0;

    // Handle complex types
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
        // Fallback to BasePart __newindex for properties not handled here
        luaL_getmetatable(L, "BasePartMeta");
        lua_getfield(L, -1, "__newindex");
        lua_remove(L, -2); // remove metatable, keep __newindex
        lua_pushvalue(L, 1); // userdata
        lua_pushvalue(L, 2); // key
        lua_pushvalue(L, 3); // value
        lua_call(L, 3, 0);
    }

    return 0;
}

void Part_Bind(lua_State* L) {
    luaL_newmetatable(L, "PartMeta");
    lua_pushcfunction(L, Part_index, "__index");
    lua_setfield(L, -2, "__index");
    lua_pushcfunction(L, Part_newindex, "__newindex");
    lua_setfield(L, -2, "__newindex");

    // Equality by underlying pointer
    lua_pushcfunction(L, Lua_UserdataPtrEq, "__eq");
    lua_setfield(L, -2, "__eq");

    // Inherit from BasePart
    luaL_getmetatable(L, "BasePartMeta");
    lua_setmetatable(L, -2);

    lua_pop(L, 1);
}

bool Part::IsA(const std::string& className) const {
    return this->ClassName == className || BasePart::IsA(className);
}
