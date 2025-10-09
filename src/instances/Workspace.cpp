#include "Workspace.h"
#include "BasePart.h"
#include "DataModel.h"
#include "Part.h"
#include <cmath>

Workspace::Workspace() : Instance("Workspace") {
    Name = "Workspace";
}

//Lua bindings
static int Workspace_index(lua_State* L) {
    Workspace** pws = (Workspace**)luaL_checkudata(L, 1, "WorkspaceMeta");
    Workspace* ws = *pws;
    const char* key = luaL_checkstring(L, 2);

    //Check for properties
    if (strcmp(key, "Name") == 0) {
        lua_pushstring(L, ws->Name.c_str());
        return 1;
    } else if (strcmp(key, "Gravity") == 0) {
        Vector3Game* v = (Vector3Game*)lua_newuserdata(L, sizeof(Vector3Game));
        *v = ws->Gravity;
        luaL_getmetatable(L, "Vector3Meta");
        lua_setmetatable(L, -2);
        return 1;
    } else if (strcmp(key, "Parent") == 0) {
        if (ws->Parent) {
            //Return DataModel parent
            lua_getglobal(L, "game");
            return 1;
        }
        lua_pushnil(L);
        return 1;
    }

    //Check for children by name
    auto child = ws->FindFirstChild(std::string(key));
    if (child.has_value()) {
        Part** udata = (Part**)lua_newuserdata(L, sizeof(Part*));
        *udata = dynamic_cast<Part*>(child.value());
        luaL_getmetatable(L, "PartMeta");
        lua_setmetatable(L, -2);
        return 1;
    }

    //Check for methods in metatable
    luaL_getmetatable(L, "WorkspaceMeta");
    lua_getfield(L, -1, key);
    if (!lua_isnil(L, -1)) {
        lua_remove(L, -2);
        return 1;
    }

    lua_pop(L, 2);
    lua_pushnil(L);
    return 1;
}

static int Workspace_newindex(lua_State* L) {
    Workspace** pws = (Workspace**)luaL_checkudata(L, 1, "WorkspaceMeta");
    Workspace* ws = *pws;
    const char* key = luaL_checkstring(L, 2);

    if (strcmp(key, "Gravity") == 0) {
        Vector3Game* v = (Vector3Game*)luaL_checkudata(L, 3, "Vector3Meta");
        ws->Gravity = *v;
    }

    return 0;
}

static int Workspace_GetChildren(lua_State* L) {
    Workspace** pws = (Workspace**)luaL_checkudata(L, 1, "WorkspaceMeta");
    Workspace* ws = *pws;

    auto children = ws->GetChildren();

    lua_newtable(L);
    for (size_t i = 0; i < children.size(); i++) {
        Part** udata = (Part**)lua_newuserdata(L, sizeof(Part*));
        *udata = dynamic_cast<Part*>(children[i]);
        luaL_getmetatable(L, "PartMeta");
        lua_setmetatable(L, -2);
        lua_rawseti(L, -2, i + 1);
    }

    return 1;
}

void Workspace::Bind(lua_State* L) {
    luaL_newmetatable(L, "WorkspaceMeta");

    lua_pushcfunction(L, Workspace_index, "__index");
    lua_setfield(L, -2, "__index");
    lua_pushcfunction(L, Workspace_newindex, "__newindex");
    lua_setfield(L, -2, "__newindex");

    //Add methods
    lua_pushcfunction(L, Workspace_GetChildren, "GetChildren");
    lua_setfield(L, -2, "GetChildren");

    lua_pop(L, 1);

    //Create global 'workspace' variable
    DataModel* dm = DataModel::GetInstance();
    Workspace* ws = dm->WorkspaceService;
    if (ws) {
        Workspace** udata = (Workspace**)lua_newuserdata(L, sizeof(Workspace*));
        *udata = ws;
        luaL_getmetatable(L, "WorkspaceMeta");
        lua_setmetatable(L, -2);
        lua_setglobal(L, "workspace");
    }
}