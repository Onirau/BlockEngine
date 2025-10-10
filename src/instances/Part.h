#pragma once
#include <iostream>
#include <string>

#include "BasePart.h"

extern const char *validShapes[];

/**
 * @class Part
 * @brief A physical 3D part that can be placed in the game world
 *
 * @description
 * Part is the most basic building block in BlockEngine. It represents a
 * physical object.
 *
 *
 * @inherits BasePart
 *
 * @example
 * ```lua
 * -- Create a platform
 * local platform = Instance.new("Part")
 * platform.Size = Vector3.new(20, 1, 20)
 * platform.Position = Vector3.new(0, 0, 0)
 * platform.Anchored = true
 * platform.Parent = workspace
 * ```
 */
struct Part : public BasePart {
    std::string Shape = "Wedge";

    Part();

    Part(const std::string &name, const Vector3Game &position,
         const Vector3Game &size, const Color3 &color, bool anchored,
         std::string shape = "Wedge");

    virtual bool IsA(const std::string &className) const;
};

void Part_Bind(lua_State *L);
