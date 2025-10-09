#include "Instance.h"
#include "../core/LuaClassBinder.h"
#include "../core/LuaBindings.h"
#include "../instances/Part.h"

//This is now handled by LuaClassBinder::GenericConstructor
//But we keep this for backward compatibility if needed
static int Instance_new(lua_State* L) {
    const char* className = luaL_checkstring(L, 1);

    //Try to use LuaClassBinder's constructor system
    auto* desc = LuaClassBinder::GetDescriptor(className);
    if (desc && desc->constructor) {
        Instance* inst = desc->constructor();
        LuaClassBinder::PushInstance(L, inst);
        return 1;
    }

    //Fallback for backward compatibility
    if (strcmp(className, "Part") == 0) {
        Part* p = new Part();
        if (LuaBindings::g_instances) {
            LuaBindings::g_instances->push_back(p);
        }
        LuaClassBinder::PushInstance(L, p);
        return 1;
    }

    luaL_error(L, "Unknown instance type '%s'", className);
    return 0;
}

void Datatype_Instance_Bind(lua_State* L) {
    lua_newtable(L);

    lua_pushcfunction(L, Instance_new, "new");
    lua_setfield(L, -2, "new");

    lua_setglobal(L, "Instance");
}