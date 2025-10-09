#pragma once
#include <climits>
#include <cstring>

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


    //-- Events --//

    //-- Methods --//

    virtual ~LuaSourceContainer() = default;

    virtual bool IsA(const std::string& className) const;
};

//Binding

void LuaSourceContainer_Bind(lua_State* L);
