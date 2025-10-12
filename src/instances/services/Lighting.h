#pragma once

#include "../../datatypes/Vector3.h"
#include "../Instance.h"

#include <lua.h>
#include <luacode.h>
#include <lualib.h>

/**
 * @class Lighting
 * @brief Controls Global Lighting
 *
 * @description
 *
 * @inherits Instance
 *
 */
struct Lighting : public Instance {

    Lighting();
    virtual ~Lighting() = default;

    // Lua bindings
    static void Bind(lua_State *L);
};