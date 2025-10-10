#pragma once

#include "LuaSourceContainer.h"

/**
 * @class ModuleScript
 * @brief A reusable script that returns a module table or value
 *
 * @description
 * ModuleScript contains Lua code that returns a value
 * which can be required by other scripts.
 *
 * @inherits LuaSourceContainer
 *
 * @example
 * ```lua
 * -- In ModuleScript
 * local module = {}
 * function module.greet(name)
 *     return "Hello, " .. name
 * end
 * return module
 *
 * -- In another Script
 * local myModule = require(script.Parent.ModuleScript)
 * print(myModule.greet("World"))
 * ```
 */
struct ModuleScript : public LuaSourceContainer {
    //-- Properties --//

    /**
     * @property ModuleRef
     * @type int
     * @default LUA_NOREF
     * @description Reference to the cached module value in the Lua registry
     */
    int ModuleRef = LUA_NOREF;

    /**
     * @property Loaded
     * @type bool
     * @default false
     * @description Whether the module has been loaded and cached
     */
    bool Loaded = false;

    //-- Methods --//
    ModuleScript();
    virtual ~ModuleScript();

    /**
     * @method Require
     * @param L lua_State
     * @returns int
     * @description Loads and returns the module value
     *
     * @example
     * ```lua
     * local module = require(game.ServerScriptService.MyModule)
     * ```
     */
    int Require(lua_State *L);

    virtual bool IsA(const std::string &className) const override;
};

void ModuleScript_Bind(lua_State *L);