#include "LuaBindings.h"
#include "../datatypes/Instance.h"
#include "../datatypes/Vector3.h"
#include "../datatypes/Color3.h"

#include "../instances/Part.h"
#include "../instances/DataModel.h"
#include "../instances/Workspace.h"

static int l_Signal_Connect(lua_State* L) {
    Signal* sig = *(Signal**)luaL_checkudata(L, 1, "Signal");
    luaL_checktype(L, 2, LUA_TFUNCTION);

    sig->ConnectLua(L, 2);
    return 0;
}

static int l_Signal_Fire(lua_State* L) {
    Signal* sig = *(Signal**)luaL_checkudata(L, 1, "Signal");

    Instance* inst = nullptr;
    if (lua_isuserdata(L, 2)) {
        inst = *(Instance**)lua_touserdata(L, 2);
    }

    sig->Fire(inst);
    return 0;
}

static int l_Signal_DisconnectAll(lua_State* L) {
    Signal* sig = *(Signal**)luaL_checkudata(L, 1, "Signal");
    sig->DisconnectAll();
    return 0;
}


static bool userdata_is_meta(lua_State* L, int idx, const char* meta) {
    if (!lua_isuserdata(L, idx)) return false;
    if (!lua_getmetatable(L, idx)) return false;//pushes metatable
    luaL_getmetatable(L, meta);//pushes registered metatable
    bool eq = lua_rawequal(L, -1, -2);
    lua_pop(L, 2);
    return eq;
}

static int l_Instance_IsA(lua_State* L) {
    Instance* inst = nullptr;

    if (userdata_is_meta(L, 1, "Instance")) {
        inst = *(Instance**)lua_touserdata(L, 1);
    } else if (userdata_is_meta(L, 1, "BasePartMeta")) {
        inst = *(BasePart**)lua_touserdata(L, 1);
    } else if (userdata_is_meta(L, 1, "PartMeta")) {
        inst = *(Part**)lua_touserdata(L, 1);
    }

    if (!inst) {
        luaL_typeerrorL(L, 1, "Instance or derived userdata");
        return 0;
    }

    const char* className = luaL_checkstring(L, 2);
    lua_pushboolean(L, inst->IsA(className));
    return 1;
}

void Lua_RegisterSignal(lua_State* L) {
    luaL_newmetatable(L, "Signal");

    lua_pushcfunction(L, l_Signal_Connect, "Connect");
    lua_setfield(L, -2, "Connect");

    lua_pushcfunction(L, l_Instance_IsA, "IsA");
    lua_setfield(L, -2, "IsA");

    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");

    lua_pop(L, 1);
}


namespace LuaBindings {
std::vector<BasePart*>* g_instances = nullptr;
Camera3D* gg_camera = nullptr;

int Lua_SetCameraPos(lua_State* L) {
    float x = (float)lua_tonumber(L, 1);
    float y = (float)lua_tonumber(L, 2);
    float z = (float)lua_tonumber(L, 3);

    gg_camera->position = {x, y, z};

    return 0;
}


void RegisterScriptBindings(lua_State* L, std::vector<BasePart*>& parts, Camera3D& g_camera) {
    g_instances = &parts;
    gg_camera = &g_camera;

    //Create Engine table
    lua_newtable(L);
    lua_pushcfunction(L, Lua_SetCameraPos, "SetCameraPos");
    lua_setfield(L, -2, "SetCameraPos");
    lua_setglobal(L, "Engine");

    //Register datatypes
    Datatype_Instance_Bind(L);
    Vector3Game_Bind(L);
    Color3_Bind(L);
    Task_Bind(L);

    //Register instance system
    Class_Instance_Bind(L);
    BasePart_Bind(L);
    Part_Bind(L);

    //Register DataModel and services
    DataModel::Bind(L);
    Workspace::Bind(L);

    //Register signals
    Lua_RegisterSignal(L);
}
}//namespace LuaBindings

//Compare two userdata values that wrap pointers; if both are userdata
//and share the same underlying pointer value (after unwrapping known
//metatables to Instance*), return true. Otherwise false. If either side
//is not userdata, return false (Lua will not fall back to raw compare
//for __eq when a metamethod exists).
static Instance* lua_unwrap_instance_any(lua_State* L, int idx) {
    //Try most common metatables first
    if (userdata_is_meta(L, idx, "Instance")) {
        return *(Instance**)lua_touserdata(L, idx);
    }
    if (userdata_is_meta(L, idx, "BasePartMeta")) {
        return *(BasePart**)lua_touserdata(L, idx);
    }
    if (userdata_is_meta(L, idx, "PartMeta")) {
        return *(Part**)lua_touserdata(L, idx);
    }
    return nullptr;
}

int Lua_UserdataPtrEq(lua_State* L) {
    Instance* a = lua_unwrap_instance_any(L, 1);
    Instance* b = lua_unwrap_instance_any(L, 2);
    lua_pushboolean(L, a != nullptr && a == b);
    return 1;
}