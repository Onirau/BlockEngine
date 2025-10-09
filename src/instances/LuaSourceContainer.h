#pragma once
#include <climits>
#include <cstring>
#include <string>

#include "../datatypes/Vector3.h"
#include "../datatypes/Color3.h"

#include "../core/Signal.h"

#include "Instance.h"

#include "../../luau/VM/include/lua.h"
#include "../../luau/VM/include/lualib.h"
#include "../../luau/Compiler/include/luacode.h"

struct LuaSourceContainer : public Instance {
    //-- Properties --//
    std::string Name = ClassName;

    //Script properties
    bool Enabled = true;
    std::string Source = "";
    std::string SourcePath = "";

    //-- Events --//

    //-- Methods --//
    LuaSourceContainer(const std::string& className = "LuaSourceContainer");
    virtual ~LuaSourceContainer() = default;

    //Execute the script (reads from Source or SourcePath based on priority)
    bool Execute(lua_State* L);

    //Load source from file path
    bool LoadFromPath();

    virtual bool IsA(const std::string& className) const;
};

//Binding
void LuaSourceContainer_Bind(lua_State* L);