#include "Script.h"
#include "../core/LuaBindings.h"
#include "../core/LuaClassBinder.h"

Script::Script() : LuaSourceContainer("Script") { Name = "Script"; }

bool Script::IsA(const std::string &className) const {
    return this->ClassName == className || LuaSourceContainer::IsA(className);
}

void Script_Bind(lua_State *L) {
    LuaClassBinder::RegisterClass("Script", "LuaSourceContainer");

    // Set constructor
    LuaClassBinder::SetConstructor("Script", []() -> Instance * {
        Script *script = new Script();
        return script;
    });
}