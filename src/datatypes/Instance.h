#pragma once
#include <string.h>
#include <vector>
#include <raylib.h>

#include "../../luau/VM/include/lua.h"
#include "../../luau/VM/include/lualib.h"
#include "../../luau/Compiler/include/luacode.h"

#include "../core/LuaClassBinder.h"
#include "../core/LuaBindings.h"

#include "../instances/Part.h"

extern std::vector<BasePart*> g_instances;
void Datatype_Instance_Bind(lua_State* L);
