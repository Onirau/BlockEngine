#pragma once

#include "LuaSourceContainer.h"

/**
 * @class Script
 * @brief A container for executable Lua code
 *
 * @description
 * Script holds Lua code that runs automatically when the game starts.
 * Unlike ModuleScript, it doesn't return a value and is used for game
 * logic and behavior.
 *
 * @inherits LuaSourceContainer
 *
 */
struct Script : public LuaSourceContainer {
    //-- Properties --//
    // RunContext could be added here for Legacy/Client/Server/Plugin

    //-- Methods --//
    Script();
    virtual ~Script() = default;

    virtual bool IsA(const std::string &className) const override;
};

void Script_Bind(lua_State *L);