#pragma once

#include <vector>

#include "core/Camera.h"
#include "core/LuaBindings.h"
#include "core/Renderer.h"
#include "instances/DataModel.h"
#include "instances/services/Lighting.h"
#include "instances/services/Workspace.h"

// Globals
extern std::vector<BasePart *> g_instances;
extern lua_State *L_main;
extern Camera g_camera;