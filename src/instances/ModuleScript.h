#pragma once

#include "LuaSourceContainer.h"

struct ModuleScript : public LuaSourceContainer {
    //-- Properties --//
    int ModuleRef = LUA_NOREF;//Reference to the returned module table/value
    bool Loaded = false;

    //-- Methods --//
    ModuleScript();
    virtual ~ModuleScript();

    //Load and return the module
    int Require(lua_State* L);

    virtual bool IsA(const std::string& className) const override;
};

void ModuleScript_Bind(lua_State* L);