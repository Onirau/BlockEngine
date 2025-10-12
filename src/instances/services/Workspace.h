#pragma once

#include "../../Global.h"
#include "../../datatypes/Vector3.h"
#include "../Instance.h"


#include <lua.h>
#include <luacode.h>
#include <lualib.h>

/**
 * @class Workspace
 * @brief The main container for all physical 3D objects in the game
 *
 * @description
 * Workspace is a service that contains all 3D parts, models, and other
 * physical objects in the game world, to be rendered; and be physically
 * simulated.
 *
 * @inherits Instance
 *
 */
struct Workspace : public Instance {
    /**
     * @property Gravity
     * @type Vector3
     * @default Vector3.new(0, -196.2, 0)
     * @description The acceleration due to gravity applied to all dynamic parts
     */
    Vector3Game Gravity = Vector3Game(0, -196.2f, 0);

    /**
     * @property CurrentCamera
     * @type Camera | nil
     * @description The active camera for rendering the Workspace
     */
    Camera *CurrentCamera = nullptr;

    Workspace();
    virtual ~Workspace() = default;

    // Lua bindings
    static void Bind(lua_State *L);
};