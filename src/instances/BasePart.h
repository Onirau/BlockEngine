#pragma once
#include <climits>
#include <cstring>

#include "../datatypes/Color3.h"
#include "../datatypes/Vector3.h"

#include "../core/Signal.h"

#include "Instance.h"

#include <lua.h>
#include <luacode.h>
#include <lualib.h>

/**
 * @class BasePart
 * @brief Base class for all physical parts in the game world
 *
 * @description
 * BasePart provides the foundation for all 3D objects that can exist in the
 * workspace. It handles physics properties like position, rotation, velocity,
 * and collision. All parts inherit from this class and gain access to physical
 * simulation capabilities.
 *
 * @inherits Instance
 *
 * @example
 * ```lua
 * -- BasePart is abstract, use Part or other subclasses
 * local part = Instance.new("Part")
 * part.Position = Vector3.new(0, 10, 0)
 * part.Anchored = false
 * ```
 */
struct BasePart : public Instance {
    //-- Properties --//
    std::string Name = ClassName;

    /**
     * @property Position
     * @type Vector3
     * @default Vector3.new(0, 0.5, 0)
     * @description The 3D position of the part in world space
     */
    Vector3Game Position = Vector3Game{0, 0.5, 0};

    /**
     * @property Rotation
     * @type Vector3
     * @default Vector3.new(0, 0, 0)
     * @description The rotation angles in degrees (X, Y, Z)
     */
    Vector3Game Rotation = Vector3Game{0, 0, 0};

    /**
     * @property Size
     * @type Vector3
     * @default Vector3.new(4, 1, 2)
     * @description The dimensions of the part
     */
    Vector3Game Size = Vector3Game{4, 1, 2};

    /**
     * @property Velocity
     * @type Vector3
     * @default Vector3.new(0, 0, 0)
     * @description The current velocity of the part
     */
    Vector3Game Velocity = Vector3Game{0, 0, 0};

    /**
     * @property RotationVelocity
     * @type Vector3
     * @default Vector3.new(0, 0, 0)
     * @description The angular velocity of the part
     */
    Vector3Game RotationVelocity = Vector3Game{0, 0, 0};

    /**
     * @property Anchored
     * @type bool
     * @default true
     * @description When true, the part is not affected by physics
     */
    bool Anchored = true;

    /**
     * @property CanCollide
     * @type bool
     * @default true
     * @description When true, the part can collide with other parts
     */
    bool CanCollide = true;

    /**
     * @property CanQuery
     * @type bool
     * @default true
     * @description When true, the part can be detected by raycasts
     */
    bool CanQuery = true;

    /**
     * @property CanTouch
     * @type bool
     * @default true
     * @description When true, the part can trigger touch events
     */
    bool CanTouch = true;

    /**
     * @property Mass
     * @type number
     * @readonly
     * @description The mass of the part calculated from size and material
     */
    double Mass;

    /**
     * @property Color
     * @type Color3
     * @default Color3.new(163/255, 162/255, 165/255)
     * @description The color of the part
     */
    Color3 Color = Color3{163.f / 255, 162.f / 255, 165.f / 255};

    /**
     * @property CastShadow
     * @type bool
     * @default true
     * @description When true, the part casts shadows
     */
    bool CastShadow = true;

    /**
     * @property Transparency
     * @type number
     * @default 0
     * @description The transparency of the part from 0 (opaque) to 1
     * (invisible)
     */
    float Transparency = 0;

    //-- Events --//

    /**
     * @event Touched
     * @param otherPart BasePart
     * @description Fires when another part touches this part
     *
     * @example
     * ```lua
     * part.Touched:Connect(function(otherPart)
     *     print("Touched by:", otherPart.Name)
     * end)
     * ```
     */
    Signal Touched;

    /**
     * @event TouchEnded
     * @param otherPart BasePart
     * @description Fires when another part stops touching this part
     */
    Signal TouchEnded;

    //-- Methods --//

    virtual ~BasePart() = default;

    /**
     * @method GetMass
     * @returns number
     * @description Returns the calculated mass of the part
     *
     * @example
     * ```lua
     * local mass = part:GetMass()
     * print("Mass:", mass)
     * ```
     */
    double GetMass();

    virtual bool IsA(const std::string &className) const;
};

// Binding

void BasePart_Bind(lua_State *L);