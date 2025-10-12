#include "Lighting.h"
#include "../../core/LuaClassBinder.h"
#include "../BasePart.h"
#include "../DataModel.h"
#include "../Part.h"
#include <cmath>

Lighting::Lighting() : Instance("Lighting") { Name = "Lighting"; }

void Lighting::Bind(lua_State *L) {
    LuaClassBinder::RegisterClass("Lighting", "Lighting");
}