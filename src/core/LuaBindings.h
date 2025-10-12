#pragma once

#include <vector>

#include <lua.h>
#include <luacode.h>
#include <lualib.h>

#include "../datatypes/Color3.h"
#include "../datatypes/Task.h"
#include "../datatypes/Vector3.h"

#include "../instances/BasePart.h"
#include "../instances/Instance.h"
#include "../instances/LuaSourceContainer.h"
#include "../instances/ModuleScript.h"
#include "../instances/Part.h"
#include "../instances/Script.h"

#include "../Global.h"
#include "Signal.h"

namespace LuaBindings {
extern std::vector<BasePart *> *g_instances;
extern Camera *gg_camera;

int Lua_SpawnPart(lua_State *L);
int Lua_SetCameraPos(lua_State *L);

void RegisterScriptBindings(lua_State *L, std::vector<BasePart *> &parts,
                            Camera &g_camera);
} // namespace LuaBindings

int Lua_UserdataPtrEq(lua_State *L);