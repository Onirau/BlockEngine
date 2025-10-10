#pragma once
#include <cmath>
#include <cstring>
#include <ostream>
#include <raylib.h>

#include "../../luau/Compiler/include/luacode.h"
#include "../../luau/VM/include/lua.h"
#include "../../luau/VM/include/lualib.h"

/**
 * @class Vector3
 * @brief Represents a 3D vector with x, y, and z components
 * @description Vector3 is a fundamental datatype for representing positions,
 * directions, and sizes in 3D space. It supports common vector operations
 * including arithmetic, magnitude calculation, normalization, and
 * interpolation.
 * @example
 * ```lua
 * -- Create a new vector
 * local pos = Vector3.new(10, 5, 0)
 *
 * -- Use predefined constants
 * local up = Vector3.yAxis
 *
 * -- Vector arithmetic
 * local result = pos + Vector3.new(0, 10, 0)
 *
 * -- Get unit vector
 * local direction = pos.Unit
 *
 * -- Interpolate between vectors
 * local midpoint = pos:Lerp(Vector3.zero, 0.5)
 * ```
 */
struct Vector3Game {
    /**
     * @property X
     * @type number
     * @description The x-coordinate of the vector
     */
    float x;

    /**
     * @property Y
     * @type number
     * @description The y-coordinate of the vector
     */
    float y;

    /**
     * @property Z
     * @type number
     * @description The z-coordinate of the vector
     */
    float z;

    Vector3Game() {
        x = 0;
        y = 0;
        z = 0;
    }
    Vector3Game(float X, float Y, float Z) {
        x = X;
        y = Y;
        z = Z;
    }

    ::Vector3 toRaylib() { return {x, y, z}; }
    Vector3Game fromRaylib(Vector3 v3) { return {v3.x, v3.y, v3.z}; }

    Vector3Game operator+(const Vector3Game &v) const {
        return {x + v.x, y + v.y, z + v.z};
    }
    Vector3Game operator-(const Vector3Game &v) const {
        return {x - v.x, y - v.y, z - v.z};
    }
    Vector3Game operator-() const { return {-x, -y, -z}; }
    Vector3Game operator*(float s) const { return {x * s, y * s, z * s}; }
    Vector3Game operator/(float s) const { return {x / s, y / s, z / s}; }

    /**
     * @property Magnitude
     * @type number
     * @description The length of the vector
     * @readonly
     * @example
     * ```lua
     * local vec = Vector3.new(3, 4, 0)
     * print(vec.Magnitude) -- 5
     * ```
     */
    float magnitude() const { return sqrtf(x * x + y * y + z * z); }

    /**
     * @property Unit
     * @type Vector3
     * @description A normalized copy of the vector with magnitude of 1
     * @readonly
     * @example
     * ```lua
     * local vec = Vector3.new(10, 0, 0)
     * local unit = vec.Unit
     * print(unit.Magnitude) -- 1
     * ```
     */
    Vector3Game unit() {
        float mag = magnitude();
        if (mag > 0.00001f)
            return *this / mag;
        return {0, 0, 0};
    }

    /**
     * @method Abs
     * @returns Vector3
     * @description Returns a Vector3 with the absolute value of each component
     * @example
     * ```lua
     * local vec = Vector3.new(-5, -10, 3)
     * local abs = vec:Abs() -- Vector3(5, 10, 3)
     * ```
     */
    Vector3Game abs() const { return {fabsf(x), fabsf(y), fabsf(z)}; }

    /**
     * @method Ceil
     * @returns Vector3
     * @description Returns a Vector3 with each component rounded up to the
     * nearest integer
     * @example
     * ```lua
     * local vec = Vector3.new(1.2, 2.7, 3.1)
     * local ceiled = vec:Ceil() -- Vector3(2, 3, 4)
     * ```
     */
    Vector3Game ceil() const {
        return {float(int(x + 1)), float(int(y + 1)), float(int(z + 1))};
    }

    /**
     * @method Floor
     * @returns Vector3
     * @description Returns a Vector3 with each component rounded down to the
     * nearest integer
     * @example
     * ```lua
     * local vec = Vector3.new(1.8, 2.3, 3.9)
     * local floored = vec:Floor() -- Vector3(1, 2, 3)
     * ```
     */
    Vector3Game floor() const {
        return {float(int(x)), float(int(y)), float(int(z))};
    }

    /**
     * @method Dot
     * @param v Vector3 - The other vector
     * @returns number
     * @description Returns the dot product of this vector and another vector
     * @example
     * ```lua
     * local a = Vector3.new(1, 0, 0)
     * local b = Vector3.new(0, 1, 0)
     * print(a:Dot(b)) -- 0 (perpendicular)
     * ```
     */
    float dot(const Vector3Game &v) const {
        return x * v.x + y * v.y + z * v.z;
    }

    /**
     * @method Cross
     * @param v Vector3 - The other vector
     * @returns Vector3
     * @description Returns the cross product of this vector and another vector
     * @example
     * ```lua
     * local a = Vector3.xAxis
     * local b = Vector3.yAxis
     * local cross = a:Cross(b) -- Returns Vector3.zAxis
     * ```
     */
    Vector3Game cross(const Vector3Game &v) const {
        return {y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x};
    }

    /**
     * @method Lerp
     * @param goal Vector3 - The target vector to interpolate towards
     * @param alpha number - The interpolation factor (0-1)
     * @returns Vector3
     * @description Returns a Vector3 linearly interpolated between this vector
     * and the goal
     * @example
     * ```lua
     * local start = Vector3.new(0, 0, 0)
     * local finish = Vector3.new(10, 10, 10)
     * local mid = start:Lerp(finish, 0.5) -- Vector3(5, 5, 5)
     * ```
     */
    Vector3Game lerp(const Vector3Game &goal, float alpha) const {
        return *this + (goal - *this) * alpha;
    }

    /**
     * @method FuzzyEq
     * @param v Vector3 - The other vector to compare
     * @returns bool
     * @description Returns true if the magnitudes of both vectors are
     * approximately equal within epsilon
     * @example
     * ```lua
     * local a = Vector3.new(1, 0, 0)
     * local b = Vector3.new(0.99999, 0, 0)
     * print(a:FuzzyEq(b)) -- true
     * ```
     */
    bool fuzzyequal(const Vector3Game &v, float epsilon = 1e-5) const {
        return fabsf(magnitude()) - fabsf(v.magnitude()) < epsilon;
    }
};

void Vector3Game_Bind(lua_State *L);