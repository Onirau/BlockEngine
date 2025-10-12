#include "Lighting.h"
#include "../../core/Constants.h"
#include "../../core/LuaClassBinder.h"
#include "../BasePart.h"
#include "../DataModel.h"
#include "../Part.h"
#include <cmath>

Lighting::Lighting() : Instance("Lighting") { Name = "Lighting"; }

Vector3Game Lighting::GetSunDirection() const {
    // Convert clock time (0-24) to radians
    // 0 = midnight, 6 = sunrise, 12 = noon, 18 = sunset
    float timeRadians = (ClockTime / 24.0f) * Constants::TWO_PI_F;

    // Convert geographic latitude to radians
    float latRadians = GeographicLatitude * Constants::DEG_TO_RAD;

    // Calculate sun elevation angle
    // At noon (12:00), sun is highest; at midnight (0:00/24:00), lowest
    float elevation = std::sin(timeRadians) * std::cos(latRadians);

    // Calculate sun azimuth (horizontal angle)
    float azimuth = timeRadians - Constants::HALF_PI_F;

    // Convert spherical coordinates to Cartesian direction vector
    float x = std::cos(elevation) * std::cos(azimuth);
    float y = std::sin(elevation);
    float z = std::cos(elevation) * std::sin(azimuth);

    // Normalize the vector
    Vector3Game direction(x, y, z);
    float length = std::sqrt(x * x + y * y + z * z);
    if (length > 0.0f) {
        direction.x /= length;
        direction.y /= length;
        direction.z /= length;
    }

    return direction;
}

void Lighting::Bind(lua_State *L) {
    LuaClassBinder::RegisterClass("Lighting", "Instance");

    // Ambient property
    LuaClassBinder::AddProperty(
        "Lighting", "Ambient",
        [](lua_State *L, Instance *inst) -> int {
            auto *lighting = static_cast<Lighting *>(inst);
            Color3 *c = (Color3 *)lua_newuserdata(L, sizeof(Color3));
            *c = lighting->Ambient;
            luaL_getmetatable(L, "Color3Meta");
            lua_setmetatable(L, -2);
            return 1;
        },
        [](lua_State *L, Instance *inst, int valueIdx) -> int {
            auto *lighting = static_cast<Lighting *>(inst);
            Color3 *c = (Color3 *)luaL_checkudata(L, valueIdx, "Color3Meta");
            lighting->Ambient = *c;
            return 0;
        });

    // Brightness property
    LuaClassBinder::AddProperty(
        "Lighting", "Brightness",
        [](lua_State *L, Instance *inst) -> int {
            auto *lighting = static_cast<Lighting *>(inst);
            lua_pushnumber(L, lighting->Brightness);
            return 1;
        },
        [](lua_State *L, Instance *inst, int valueIdx) -> int {
            auto *lighting = static_cast<Lighting *>(inst);
            lighting->Brightness = luaL_checknumber(L, valueIdx);
            return 0;
        });

    // ClockTime property
    LuaClassBinder::AddProperty(
        "Lighting", "ClockTime",
        [](lua_State *L, Instance *inst) -> int {
            auto *lighting = static_cast<Lighting *>(inst);
            lua_pushnumber(L, lighting->ClockTime);
            return 1;
        },
        [](lua_State *L, Instance *inst, int valueIdx) -> int {
            auto *lighting = static_cast<Lighting *>(inst);
            lighting->ClockTime = luaL_checknumber(L, valueIdx);
            return 0;
        });

    // GeographicLatitude property
    LuaClassBinder::AddProperty(
        "Lighting", "GeographicLatitude",
        [](lua_State *L, Instance *inst) -> int {
            auto *lighting = static_cast<Lighting *>(inst);
            lua_pushnumber(L, lighting->GeographicLatitude);
            return 1;
        },
        [](lua_State *L, Instance *inst, int valueIdx) -> int {
            auto *lighting = static_cast<Lighting *>(inst);
            lighting->GeographicLatitude = luaL_checknumber(L, valueIdx);
            return 0;
        });

    // GetSunDirection method
    LuaClassBinder::AddMethod(
        "Lighting", "GetSunDirection",
        [](lua_State *L, ::Instance *inst) -> int {
            auto *lighting = static_cast<Lighting *>(inst);
            Vector3Game direction = lighting->GetSunDirection();

            Vector3Game *v =
                (Vector3Game *)lua_newuserdata(L, sizeof(Vector3Game));
            *v = direction;
            luaL_getmetatable(L, "Vector3Meta");
            lua_setmetatable(L, -2);
            return 1;
        });
}