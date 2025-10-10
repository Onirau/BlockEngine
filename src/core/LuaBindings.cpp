#include "LuaBindings.h"
#include "../datatypes/Color3.h"
#include "../datatypes/Vector3.h"

#include "../instances/DataModel.h"
#include "../instances/LuaSourceContainer.h"
#include "../instances/ModuleScript.h"
#include "../instances/Part.h"
#include "../instances/Script.h"
#include "../instances/Workspace.h"

#include "LuaClassBinder.h"

// Legacy signal binding (keep for now)
static int l_Signal_Connect(lua_State *L) {
    Signal *sig = *(Signal **)luaL_checkudata(L, 1, "Signal");
    luaL_checktype(L, 2, LUA_TFUNCTION);

    sig->ConnectLua(L, 2);
    return 0;
}

static int l_Signal_Fire(lua_State *L) {
    Signal *sig = *(Signal **)luaL_checkudata(L, 1, "Signal");

    Instance *inst = nullptr;
    if (lua_isuserdata(L, 2)) {
        inst = LuaClassBinder::CheckInstance(L, 2);
    }

    sig->Fire(inst);
    return 0;
}

static int l_Signal_DisconnectAll(lua_State *L) {
    Signal *sig = *(Signal **)luaL_checkudata(L, 1, "Signal");
    sig->DisconnectAll();
    return 0;
}

void Lua_RegisterSignal(lua_State *L) {
    luaL_newmetatable(L, "Signal");

    lua_pushcfunction(L, l_Signal_Connect, "Connect");
    lua_setfield(L, -2, "Connect");

    lua_pushcfunction(L, l_Signal_Fire, "Fire");
    lua_setfield(L, -2, "Fire");

    lua_pushcfunction(L, l_Signal_DisconnectAll, "DisconnectAll");
    lua_setfield(L, -2, "DisconnectAll");

    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");

    lua_pop(L, 1);
}

namespace LuaBindings {
std::vector<BasePart *> *g_instances = nullptr;
Camera3D *gg_camera = nullptr;

int Lua_SetCameraPos(lua_State *L) {
    float x = (float)lua_tonumber(L, 1);
    float y = (float)lua_tonumber(L, 2);
    float z = (float)lua_tonumber(L, 3);

    gg_camera->position = {x, y, z};

    return 0;
}

void RegisterScriptBindings(lua_State *L, std::vector<BasePart *> &parts,
                            Camera3D &g_camera) {
    g_instances = &parts;
    gg_camera = &g_camera;

    // Create Engine table
    lua_newtable(L);
    lua_pushcfunction(L, Lua_SetCameraPos, "SetCameraPos");
    lua_setfield(L, -2, "SetCameraPos");
    lua_setglobal(L, "Engine");

    // Register datatypes
    Vector3Game_Bind(L);
    Color3_Bind(L);
    Task_Bind(L);

    // Register instance classes using LuaClassBinder
    Object_Bind(L);             // Base Object Class
    Class_Instance_Bind(L);     // Instance inherits from Object
    BasePart_Bind(L);           // BasePart inherits from Instance
    Part_Bind(L);               // Part inherits from BasePart
    LuaSourceContainer_Bind(L); // LuaSourceContainer inherits from Instance
    Script_Bind(L);             // Script inherits from LuaSourceContainer
    ModuleScript_Bind(L);       // ModuleScript inherits from LuaSourceContainer

    // Register services
    DataModel::Bind(L); // Creates 'game' global
    Workspace::Bind(L); // Creates 'workspace' global

    // Register signals
    Lua_RegisterSignal(L);

    // Bind all registered classes and create metatables
    LuaClassBinder::BindAll(L);
}
} // namespace LuaBindings