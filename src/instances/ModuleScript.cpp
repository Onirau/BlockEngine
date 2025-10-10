#include "ModuleScript.h"
#include "../core/LuaBindings.h"
#include "../core/LuaClassBinder.h"
#include "../datatypes/Task.h"
#include <fstream>
#include <sstream>

ModuleScript::ModuleScript() : LuaSourceContainer("ModuleScript") {
    Name = "ModuleScript";
}

ModuleScript::~ModuleScript() {
    // Clean up the module reference if it exists
    if (ModuleRef != LUA_NOREF && LuaSourceContainer::Enabled) {
        // Note: We need the lua_State to unref, but we don't store it
    }
}

int ModuleScript::Require(lua_State *L) {
    if (!Enabled) {
        luaL_error(L, "Cannot require disabled ModuleScript '%s'",
                   Name.c_str());
        return 0;
    }

    // If already loaded, return cached result
    if (Loaded && ModuleRef != LUA_NOREF) {
        lua_rawgeti(L, LUA_REGISTRYINDEX, ModuleRef);
        return 1;
    }

    std::string scriptSource = Source;

    // If Source is empty but SourcePath is set, load from file
    if (scriptSource.empty() && !SourcePath.empty()) {
        if (!LoadFromPath()) {
            luaL_error(L, "Failed to load ModuleScript from path: %s",
                       SourcePath.c_str());
            return 0;
        }
        scriptSource = Source;
    }

    // If still empty, error
    if (scriptSource.empty()) {
        luaL_error(L, "ModuleScript '%s' has no source code", Name.c_str());
        return 0;
    }

    // Compile the script
    size_t bytecodeSize = 0;
    char *bytecode = luau_compile(scriptSource.c_str(), scriptSource.size(),
                                  nullptr, &bytecodeSize);

    if (!bytecode) {
        luaL_error(L, "Failed to compile ModuleScript '%s'", Name.c_str());
        return 0;
    }

    // Load the bytecode
    std::string chunkname = "@" + Name;
    // Prepare per-module environment and pass it to luau_load
    lua_newtable(L);              // env
    lua_newtable(L);              // mt
    lua_pushvalue(L, LUA_GLOBALSINDEX); // _G
    lua_setfield(L, -2, "__index");
    lua_setmetatable(L, -2);      // setmetatable(env, mt)

    int envIndex = lua_gettop(L);
    int loadResult = luau_load(L, chunkname.c_str(), bytecode, bytecodeSize, envIndex);
    lua_remove(L, envIndex);      // remove env, keep function
    free(bytecode);

    if (loadResult != 0) {
        const char *errMsg = lua_tostring(L, -1);
        luaL_error(L, "Error loading ModuleScript '%s': %s", Name.c_str(),
                   errMsg);
        return 0;
    }

    // Execute the module script (it should return a value)
    if (lua_pcall(L, 0, 1, 0) != LUA_OK) {
        const char *errMsg = lua_tostring(L, -1);
        luaL_error(L, "Error executing ModuleScript '%s': %s", Name.c_str(),
                   errMsg);
        return 0;
    }

    // Store the returned value in the registry
    lua_pushvalue(L, -1); // Duplicate the return value
    ModuleRef = lua_ref(L, LUA_REGISTRYINDEX);
    Loaded = true;

    // Return the module value (still on stack)
    return 1;
}

bool ModuleScript::IsA(const std::string &className) const {
    return this->ClassName == className || LuaSourceContainer::IsA(className);
}

void ModuleScript_Bind(lua_State *L) {
    LuaClassBinder::RegisterClass("ModuleScript", "LuaSourceContainer");

    // LinkedSource property (read-only, always returns empty for now)
    LuaClassBinder::AddProperty(
        "ModuleScript", "LinkedSource",
        [](lua_State *L, Instance *inst) -> int {
            lua_pushstring(L, "");
            return 1;
        },
        nullptr); // Read-only

    // Set constructor
    LuaClassBinder::SetConstructor("ModuleScript", []() -> Instance * {
        ModuleScript *module = new ModuleScript();
        return module;
    });

    // Create global require function
    lua_pushcfunction(
        L,
        [](lua_State *L) -> int {
            // Check if argument is a ModuleScript instance
            if (!lua_isuserdata(L, 1)) {
                luaL_error(L, "require expects a ModuleScript");
                return 0;
            }

            Instance *inst = LuaClassBinder::CheckInstance(L, 1);
            if (!inst->IsA("ModuleScript")) {
                luaL_error(L, "require expects a ModuleScript, got %s",
                           inst->ClassName.c_str());
                return 0;
            }

            ModuleScript *module = static_cast<ModuleScript *>(inst);
            return module->Require(L);
        },
        "require");
    lua_setglobal(L, "require");
}