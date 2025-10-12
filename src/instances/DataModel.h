#pragma once

#include "services/ServiceProvider.h"
#include <memory>

#include <lua.h>
#include <luacode.h>
#include <lualib.h>

struct Workspace;
struct Lighting;

/**
 * @class DataModel
 * @brief The root container for all game services and instances
 *
 * @description
 * DataModel represents the top-level container in the instance hierarchy.
 * It inherits from ServiceProvider and manages all game services like
 * Workspace. Typically accessed via the global `game` variable in Lua.
 *
 * @inherits ServiceProvider
 *
 * @example
 * ```lua
 * local workspace = game:GetService("Workspace")
 * local part = Instance.new("Part")
 * part.Parent = workspace
 * ```
 */
struct DataModel : public ServiceProvider {
    static DataModel *Instance;

    Workspace *WorkspaceService = nullptr;
    Lighting *LightingService = nullptr;

    DataModel();
    virtual ~DataModel();

    static DataModel *GetInstance();
    virtual ::Instance *CreateService(const std::string &serviceName) override;

    void InitializeServices();

    virtual bool IsA(const std::string &className) const override;

    static void Bind(lua_State *L);
};