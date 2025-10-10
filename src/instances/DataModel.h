#pragma once

#include "Instance.h"
#include <memory>
#include <unordered_map>

#include "../../luau/Compiler/include/luacode.h"
#include "../../luau/VM/include/lua.h"
#include "../../luau/VM/include/lualib.h"

struct Workspace;

/**
 * @class DataModel
 * @brief The root container for all game services and instances
 *
 * @description
 * DataModel represents the top-level container in the instance hierarchy.
 * It manages all game services like Workspace and provides access to them.
 * Typically accessed via the global `game` variable in Lua.
 *
 * @inherits Instance
 *
 * @example
 * ```lua
 * local workspace = game:GetService("Workspace")
 * local part = Instance.new("Part")
 * part.Parent = workspace
 * ```
 */
struct DataModel : public Instance {
    static DataModel *Instance;

    std::unordered_map<std::string, ::Instance *> Services;

    Workspace *WorkspaceService = nullptr;

    DataModel();
    virtual ~DataModel();

    /**
     * @method GetInstance
     * @internal
     * @returns DataModel
     * @description Returns the singleton DataModel instance
     */
    static DataModel *GetInstance();

    /**
     * @method GetService
     * @param serviceName string
     * @returns Instance
     * @description Gets a service by name, creating it if it doesn't exist
     *
     * @example
     * ```lua
     * local workspace = game:GetService("Workspace")
     * ```
     */
    ::Instance *GetService(const std::string &serviceName);

    /**
     * @method FindService
     * @param serviceName string
     * @returns Instance | nil
     * @description Finds a service by name without creating it
     */
    ::Instance *FindService(const std::string &serviceName);

    void RegisterService(const std::string &name, ::Instance *service);
    void InitializeServices();

    static void Bind(lua_State *L);
};