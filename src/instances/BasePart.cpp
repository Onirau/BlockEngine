#include "BasePart.h"
#include "../core/LuaClassBinder.h"
#include "../core/LuaBindings.h"

double BasePart::GetMass() {
    //Simple mass calculation based on volume
    return Size.x * Size.y * Size.z;
}

bool BasePart::IsA(const std::string& className) const {
    return this->ClassName == className || Instance::IsA(className);
}

void BasePart_Bind(lua_State* L) {
    (void)L;//Suppress unused parameter warning
    LuaClassBinder::RegisterClass("BasePart", "Instance");

    //Properties - Vector3 types
    LuaClassBinder::AddProperty("BasePart", "Position", [](lua_State* L, Instance* inst) -> int {
            auto* part = static_cast<BasePart*>(inst);
            Vector3Game* v = (Vector3Game*)lua_newuserdata(L, sizeof(Vector3Game));
            *v = part->Position;
            luaL_getmetatable(L, "Vector3Meta");
            lua_setmetatable(L, -2);
            return 1; }, [](lua_State* L, Instance* inst, int valueIdx) -> int {
            auto* part = static_cast<BasePart*>(inst);
            Vector3Game* v = (Vector3Game*)luaL_checkudata(L, valueIdx, "Vector3Meta");
            part->Position = *v;
            return 0; });

    LuaClassBinder::AddProperty("BasePart", "Rotation", [](lua_State* L, Instance* inst) -> int {
            auto* part = static_cast<BasePart*>(inst);
            Vector3Game* v = (Vector3Game*)lua_newuserdata(L, sizeof(Vector3Game));
            *v = part->Rotation;
            luaL_getmetatable(L, "Vector3Meta");
            lua_setmetatable(L, -2);
            return 1; }, [](lua_State* L, Instance* inst, int valueIdx) -> int {
            auto* part = static_cast<BasePart*>(inst);
            Vector3Game* v = (Vector3Game*)luaL_checkudata(L, valueIdx, "Vector3Meta");
            part->Rotation = *v;
            return 0; });

    LuaClassBinder::AddProperty("BasePart", "Size", [](lua_State* L, Instance* inst) -> int {
            auto* part = static_cast<BasePart*>(inst);
            Vector3Game* v = (Vector3Game*)lua_newuserdata(L, sizeof(Vector3Game));
            *v = part->Size;
            luaL_getmetatable(L, "Vector3Meta");
            lua_setmetatable(L, -2);
            return 1; }, [](lua_State* L, Instance* inst, int valueIdx) -> int {
            auto* part = static_cast<BasePart*>(inst);
            Vector3Game* v = (Vector3Game*)luaL_checkudata(L, valueIdx, "Vector3Meta");
            part->Size = *v;
            return 0; });

    LuaClassBinder::AddProperty("BasePart", "Velocity", [](lua_State* L, Instance* inst) -> int {
            auto* part = static_cast<BasePart*>(inst);
            Vector3Game* v = (Vector3Game*)lua_newuserdata(L, sizeof(Vector3Game));
            *v = part->Velocity;
            luaL_getmetatable(L, "Vector3Meta");
            lua_setmetatable(L, -2);
            return 1; }, [](lua_State* L, Instance* inst, int valueIdx) -> int {
            auto* part = static_cast<BasePart*>(inst);
            Vector3Game* v = (Vector3Game*)luaL_checkudata(L, valueIdx, "Vector3Meta");
            part->Velocity = *v;
            return 0; });

    //Properties - Color3
    LuaClassBinder::AddProperty("BasePart", "Color", [](lua_State* L, Instance* inst) -> int {
            auto* part = static_cast<BasePart*>(inst);
            Color3* c = (Color3*)lua_newuserdata(L, sizeof(Color3));
            *c = part->Color;
            luaL_getmetatable(L, "Color3Meta");
            lua_setmetatable(L, -2);
            return 1; }, [](lua_State* L, Instance* inst, int valueIdx) -> int {
            auto* part = static_cast<BasePart*>(inst);
            Color3* c = (Color3*)luaL_checkudata(L, valueIdx, "Color3Meta");
            part->Color = *c;
            return 0; });

    //Properties - Boolean
    LuaClassBinder::AddProperty("BasePart", "Anchored", [](lua_State* L, Instance* inst) -> int {
            auto* part = static_cast<BasePart*>(inst);
            lua_pushboolean(L, part->Anchored);
            return 1; }, [](lua_State* L, Instance* inst, int valueIdx) -> int {
            auto* part = static_cast<BasePart*>(inst);
            part->Anchored = lua_toboolean(L, valueIdx) != 0;
            return 0; });

    LuaClassBinder::AddProperty("BasePart", "CanCollide", [](lua_State* L, Instance* inst) -> int {
            auto* part = static_cast<BasePart*>(inst);
            lua_pushboolean(L, part->CanCollide);
            return 1; }, [](lua_State* L, Instance* inst, int valueIdx) -> int {
            auto* part = static_cast<BasePart*>(inst);
            part->CanCollide = lua_toboolean(L, valueIdx) != 0;
            return 0; });

    LuaClassBinder::AddProperty("BasePart", "CanQuery", [](lua_State* L, Instance* inst) -> int {
            auto* part = static_cast<BasePart*>(inst);
            lua_pushboolean(L, part->CanQuery);
            return 1; }, [](lua_State* L, Instance* inst, int valueIdx) -> int {
            auto* part = static_cast<BasePart*>(inst);
            part->CanQuery = lua_toboolean(L, valueIdx) != 0;
            return 0; });

    LuaClassBinder::AddProperty("BasePart", "CanTouch", [](lua_State* L, Instance* inst) -> int {
            auto* part = static_cast<BasePart*>(inst);
            lua_pushboolean(L, part->CanTouch);
            return 1; }, [](lua_State* L, Instance* inst, int valueIdx) -> int {
            auto* part = static_cast<BasePart*>(inst);
            part->CanTouch = lua_toboolean(L, valueIdx) != 0;
            return 0; });

    LuaClassBinder::AddProperty("BasePart", "CastShadow", [](lua_State* L, Instance* inst) -> int {
            auto* part = static_cast<BasePart*>(inst);
            lua_pushboolean(L, part->CastShadow);
            return 1; }, [](lua_State* L, Instance* inst, int valueIdx) -> int {
            auto* part = static_cast<BasePart*>(inst);
            part->CastShadow = lua_toboolean(L, valueIdx) != 0;
            return 0; });

    //Properties - Number
    LuaClassBinder::AddProperty("BasePart", "Transparency", [](lua_State* L, Instance* inst) -> int {
            auto* part = static_cast<BasePart*>(inst);
            lua_pushnumber(L, part->Transparency);
            return 1; }, [](lua_State* L, Instance* inst, int valueIdx) -> int {
            auto* part = static_cast<BasePart*>(inst);
            part->Transparency = (float)luaL_checknumber(L, valueIdx);
            return 0; });

    LuaClassBinder::AddProperty("BasePart", "Mass", [](lua_State* L, Instance* inst) -> int {
            auto* part = static_cast<BasePart*>(inst);
            lua_pushnumber(L, part->Mass);
            return 1; },
                                nullptr);//Read-only

    //Methods
    LuaClassBinder::AddMethod("BasePart", "GetMass",
                              [](lua_State* L, Instance* inst) -> int {
                                  auto* part = static_cast<BasePart*>(inst);
                                  lua_pushnumber(L, part->GetMass());
                                  return 1;
                              });
}