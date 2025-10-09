#include "Part.h"
#include "DataModel.h"
#include "../core/LuaClassBinder.h"
#include "../core/LuaBindings.h"
#include <algorithm>

const char* validShapes[] = {"Block", "Sphere", "Cylinder", "Wedge", "CornerWedge", nullptr};

Part::Part() : BasePart() {
    ClassName = "Part";
    Shape = "Block";
}

Part::Part(const std::string& name,
           const Vector3Game& position,
           const Vector3Game& size,
           const Color3& color,
           bool anchored,
           std::string shape)
    : BasePart() {
    Name = name;
    Position = position;
    Size = size;
    Color = color;
    Anchored = anchored;
    Shape = shape;
    ClassName = "Part";
}

bool Part::IsA(const std::string& className) const {
    return this->ClassName == className || BasePart::IsA(className);
}

void Part_Bind(lua_State* L) {
    (void)L;//Suppress unused parameter warning
    LuaClassBinder::RegisterClass("Part", "BasePart");

    //Shape property with validation
    LuaClassBinder::AddProperty("Part", "Shape", [](lua_State* L, Instance* inst) -> int {
            auto* part = static_cast<Part*>(inst);
            lua_pushstring(L, part->Shape.c_str());
            return 1; }, [](lua_State* L, Instance* inst, int valueIdx) -> int {
            auto* part = static_cast<Part*>(inst);
            const char* newShape = luaL_checkstring(L, valueIdx);
            
            // Validate shape
            bool valid = false;
            for (int i = 0; validShapes[i] != nullptr; i++) {
                if (strcmp(newShape, validShapes[i]) == 0) {
                    valid = true;
                    break;
                }
            }
            
            if (valid) {
                part->Shape = std::string(newShape);
            } else {
                luaL_error(L, "attempt to set invalid Part.Shape value of '%s'", newShape);
            }
            return 0; });

    //Set constructor
    LuaClassBinder::SetConstructor("Part", []() -> Instance* {
        Part* part = new Part();
        //Add to global instances if needed
        if (LuaBindings::g_instances) {
            LuaBindings::g_instances->push_back(part);
        }
        return part;
    });
}