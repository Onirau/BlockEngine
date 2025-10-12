#pragma once

#include "../../datatypes/Color3.h"
#include "../../datatypes/Vector3.h"
#include "../Instance.h"


#include <lua.h>
#include <luacode.h>
#include <lualib.h>

/**
 * @class Lighting
 * @brief Controls Global Lighting
 *
 * @description
 *
 * @inherits Instance
 *
 */
struct Lighting : public Instance {
    /**
     * @property Ambient
     * @type Color3
     * @default Color3.new(0, 0, 0)
     * @description The ambient light color applied to all surfaces
     */
    Color3 Ambient = Color3(0, 0, 0);

    /**
     * @property Brightness
     * @type number
     * @default 1.0
     * @description The overall brightness multiplier for lighting
     */
    float Brightness = 1.0f;

    /**
     * @property GeographicLatitude
     * @type number
     * @default 0.0
     * @description The geographic latitude used for sun position calculations
     * (in degrees)
     */
    float GeographicLatitude = 0.0f;

    /**
     * @property ClockTime
     * @type number
     * @default 0.0
     * @description The time of day in hours (0-24), used for sun position
     */
    float ClockTime = 0.0f;

    Lighting();
    virtual ~Lighting() = default;

    /**
     * @method GetSunDirection
     * @returns Vector3
     * @description Returns the direction vector of the sun.
     *
     */
    Vector3Game GetSunDirection() const;

    // Lua bindings
    static void Bind(lua_State *L);
};