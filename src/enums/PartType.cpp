#include "PartType.h"
#include "../core/EnumRegistry.h"

// Register Enum.PartType from the C++ enum names, starting at 0
static void RegisterEnum_PartType(lua_State *L) {
    RegisterEnumByNames(L, "PartType", kPartTypeNames, kPartTypeCount, 0);
}

static EnumRegistrar s_registrar_PartType(RegisterEnum_PartType);
