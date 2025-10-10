#pragma once

#include "../core/Signal.h"
#include <algorithm>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

// Forward declaration for Lua
struct lua_State;

/**
 * @class Object
 * @brief The Root class for all engine objects
 *
 * @description
 * Object is the base class for all engine objects.
 *
 * All classes in the engine inherit from Object.
 *
 */
struct Object {
    //-- Properties --//

    /**
     * @property ClassName
     * @type string
     * @readonly
     * @description The name of this object's class
     */
    std::string ClassName;

    /**
     * @property Name
     * @type string
     * @description The name of this object
     */
    std::string Name;

    //-- Events --//

    /**
     * @event Changed
     * @param propertyName string
     * @description Fires when a property of this object changes
     *
     * @example
     * ```lua
     * part.Changed:Connect(function(property)
     *     print("Property changed:", property)
     * end)
     * ```
     */
    Signal Changed;

    //-- Methods --//
    Object(const std::string &className = "Object");
    virtual ~Object();

    /**
     * @method IsA
     * @param className string
     * @returns bool
     * @description Checks if this object is an instance of the specified class
     *
     */
    virtual bool IsA(const std::string &className) const;

    /**
     * @method FirePropertyChanged
     * @param propertyName string
     * @description Triggers the Changed event for a specific property
     */
    virtual void FirePropertyChanged(const std::string &propertyName);
};

// Lua binding function declaration
void Object_Bind(lua_State *L);