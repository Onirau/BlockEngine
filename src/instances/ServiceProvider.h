#pragma once

#include "Instance.h"
#include <memory>
#include <unordered_map>

#include "../../luau/Compiler/include/luacode.h"
#include "../../luau/VM/include/lua.h"
#include "../../luau/VM/include/lualib.h"

/**
 * @class ServiceProvider
 * @brief Abstract base class for containers that provide singleton services
 *
 * @description
 * ServiceProvider is an abstract class that manages singleton service
 * instances.
 *
 * @inherits Instance
 *
 */
struct ServiceProvider : public Instance {
protected:
    /**
     * @property Services
     * @internal
     * @type table
     * @description Map of service names to service instances
     */
    std::unordered_map<std::string, ::Instance *> Services;

public:
    ServiceProvider(const std::string &className = "ServiceProvider");
    virtual ~ServiceProvider();

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
    virtual ::Instance *GetService(const std::string &serviceName);

    /**
     * @method FindService
     * @param serviceName string
     * @returns Instance | nil
     * @description Finds a service by name without creating it
     *
     * @example
     * ```lua
     * local workspace = game:FindService("Workspace")
     * if workspace then
     *     print("Workspace exists")
     * end
     * ```
     */
    ::Instance *FindService(const std::string &serviceName);

    void RegisterService(const std::string &name, ::Instance *service);
    virtual ::Instance *CreateService(const std::string &serviceName) = 0;

    virtual bool IsA(const std::string &className) const override;

    static void Bind(lua_State *L);
};