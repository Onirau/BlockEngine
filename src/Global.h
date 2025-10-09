#pragma once

#include <vector>
#include "raylib.h"

#include "instances/DataModel.h"
#include "instances/Workspace.h"
#include "core/LuaBindings.h"
#include "core/Renderer.h"

//Globals
extern std::vector<BasePart*> g_instances;
extern lua_State* L_main;
extern Camera3D g_camera;