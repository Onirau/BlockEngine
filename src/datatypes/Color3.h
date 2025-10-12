#pragma once
#include <algorithm>
#include <cmath>

#include <lua.h>
#include <luacode.h>
#include <lualib.h>

/**
 * @brief Represents a color with RGB components in the range [0, 1]
 * @description Color3 is a datatype used for working with colors. Each
 * component (red, green, blue) is stored as a floating-point value between 0
 * and 1, where 0 represents no intensity and 1 represents full intensity.
 * @example
 * ```lua
 * -- Create a red color
 * local red = Color3.new(1, 0, 0)
 *
 * -- Create from RGB values (0-255)
 * local blue = Color3.fromRGB(0, 0, 255)
 *
 * -- Create from HSV
 * local yellow = Color3.fromHSV(0.167, 1, 1)
 *
 * -- Lerp between colors
 * local purple = red:Lerp(blue, 0.5)
 * ```
 */
struct Color3 {
    /**
     * @property r
     * @type number
     * @description The red component of the color (0-1)
     * @readonly
     */
    float r;

    /**
     * @property g
     * @type number
     * @description The green component of the color (0-1)
     * @readonly
     */
    float g;

    /**
     * @property b
     * @type number
     * @description The blue component of the color (0-1)
     * @readonly
     */
    float b;

    Color3() : r(0), g(0), b(0) {}
    Color3(float red, float green, float blue)
        : r(std::clamp(red, 0.0f, 1.0f)), g(std::clamp(green, 0.0f, 1.0f)),
          b(std::clamp(blue, 0.0f, 1.0f)) {}

    /**
     * @method fromRGB
     * @param r number - Red component (0-255)
     * @param g number - Green component (0-255)
     * @param b number - Blue component (0-255)
     * @returns Color3
     * @description Creates a Color3 from RGB values in the range 0-255
     * @example
     * ```lua
     * local orange = Color3.fromRGB(255, 165, 0)
     * ```
     */
    static Color3 fromRGB(int r, int g, int b) {
        return Color3(r / 255.0f, g / 255.0f, b / 255.0f);
    }

    /**
     * @method fromHSV
     * @param h number - Hue (0-1)
     * @param s number - Saturation (0-1)
     * @param v number - Value/Brightness (0-1)
     * @returns Color3
     * @description Creates a Color3 from HSV (Hue, Saturation, Value) color
     * space
     * @example
     * ```lua
     * -- Create a bright green
     * local green = Color3.fromHSV(0.333, 1, 1)
     * ```
     */
    static Color3 fromHSV(float h, float s, float v);

    /**
     * @method Lerp
     * @param other Color3 - The target color to interpolate towards
     * @param alpha number - The interpolation factor (0-1)
     * @returns Color3
     * @description Returns a Color3 linearly interpolated between this color
     * and the target color
     * @example
     * ```lua
     * local red = Color3.new(1, 0, 0)
     * local blue = Color3.new(0, 0, 1)
     * local purple = red:Lerp(blue, 0.5)
     * ```
     */
    Color3 Lerp(const Color3 &other, float alpha) const;
};

void Color3_Bind(lua_State *L);