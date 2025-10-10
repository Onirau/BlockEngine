#pragma once

#include "../datatypes/Vector3.h"
#include "Instance.h"

#include "../../luau/Compiler/include/luacode.h"
#include "../../luau/VM/include/lua.h"
#include "../../luau/VM/include/lualib.h"

struct Workspace : public Instance {
    // Properties specific to Workspace
    Vector3Game Gravity = Vector3Game(0, -196.2f, 0);

    // Camera (if you want workspace to own it)
    Camera3D *CurrentCamera = nullptr;

    Workspace();
    virtual ~Workspace() = default;

    // Lua bindings
    static void Bind(lua_State *L);
};