#pragma once
#include <climits>
#include <cstring>
#include <string>

#include "../datatypes/Color3.h"
#include "../datatypes/Vector3.h"

#include "../core/Signal.h"

#include "Instance.h"

#include "../../luau/Compiler/include/luacode.h"
#include "../../luau/VM/include/lua.h"
#include "../../luau/VM/include/lualib.h"

/**
 * @class LuaSourceContainer
 * @brief Base class for all script types that can execute Lua code
 *
 * @description
 * LuaSourceContainer provides the foundation for Lua.
 * It handles loading and executing Lua source code from embedding
 * or a file path.
 *
 * @inherits Instance
 *
 */
struct LuaSourceContainer : public Instance {
    //-- Properties --//
    std::string Name = ClassName;

    /**
     * @property Enabled
     * @type bool
     * @default true
     * @description When true, the script can execute; when appropriate.
     */
    bool Enabled = true;

    /**
     * @property Source
     * @type string
     * @default ""
     * @description The Lua source code to execute.
     */
    std::string Source = "";

    /**
     * @property SourcePath
     * @type string
     * @default ""
     * @description File path to fetch and load source code from.
     */
    std::string SourcePath = "";

    //-- Events --//

    //-- Methods --//
    LuaSourceContainer(const std::string &className = "LuaSourceContainer");
    virtual ~LuaSourceContainer() = default;

    bool Execute(lua_State *L);
    bool LoadFromPath();

    virtual bool IsA(const std::string &className) const;
};

// Binding
void LuaSourceContainer_Bind(lua_State *L);