#pragma once

#include "LuaSourceContainer.h"

struct Script : public LuaSourceContainer {
    //-- Properties --//
    // RunContext could be added here for Legacy/Client/Server/Plugin

    //-- Methods --//
    Script();
    virtual ~Script() = default;

    virtual bool IsA(const std::string &className) const override;
};

void Script_Bind(lua_State *L);