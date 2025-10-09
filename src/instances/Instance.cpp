#include "Instance.h"
#include "../core/LuaBindings.h"
#include "DataModel.h"
#include "Workspace.h"
#include "Part.h"

Instance::Instance(const std::string& className)
    : Object(className), Name(className) {
}

Instance::~Instance() {
    Destroy();
}
//------ Attributes ------//

void Instance::SetAttribute(std::string& attribute, Attribute value) {
    for (auto& attr : Attributes) {
        if (attr.Name == attribute) {
            attr = value;

            AttributeChanged.Fire(attribute);
            return;
        }
    }

    Attributes.push_back(value);
    AttributeChanged.Fire(attribute);
}

std::optional<Attribute> Instance::GetAttribute(std::string& attribute) {
    for (auto& attr : Attributes) {
        if (attr.Name == attribute) {
            return attr;
        }
    }
    return std::nullopt;
}

std::vector<Attribute> Instance::GetAttributes() {
    return Attributes;
}

//------ Tags ------//

void Instance::AddTag(std::string& tag) {
    //TODO: implement tags
}

bool Instance::HasTag(std::string& tag) {
    //TODO: implement tag lookup
    return false;
}

void Instance::RemoveTag(std::string& tag) {
    //TODO: implement tag removal
}

//------ Tags ------//

void Instance::SetParent(Instance* newParent) {
    if (Parent == newParent)
        return;

    Instance* oldParent = Parent;

    if (Parent) {
        auto& siblings = Parent->Children;
        siblings.erase(std::remove(siblings.begin(), siblings.end(), this), siblings.end());
    }

    Parent = newParent;

    if (newParent) {
        newParent->Children.push_back(this);

        newParent->ChildAdded.Fire(this);
        newParent->DescendantAdded.Fire(this);
    }

    AncestryChanged.Fire(this);
    if (oldParent)
        oldParent->ChildRemoved.Fire(this);
}

void Instance::AddChild(Instance* child) {
    if (!child || child == this)
        return;

    child->SetParent(this);
}

void Instance::RemoveChild(Instance* child) {
    if (!child)
        return;

    auto it = std::remove(Children.begin(), Children.end(), child);
    if (it != Children.end()) {
        Children.erase(it, Children.end());
        child->Parent = nullptr;

        ChildRemoved.Fire(child);
    }
}

void Instance::Destroy() {
    Destroying.Fire(this);

    for (auto* child : Children) {
        if (child)
            child->Destroy();
    }

    Children.clear();

    if (Parent)
        Parent->RemoveChild(this);

    Parent = nullptr;
}

std::optional<Instance*> Instance::FindFirstAncestor(std::string& name) {
    Instance* current = Parent;
    while (current) {
        if (current->Name == name)
            return current;
        current = current->Parent;
    }

    return std::nullopt;
}

std::optional<Instance*> Instance::FindFirstAncestorOfClass(std::string& className) {
    Instance* current = Parent;
    while (current) {
        if (current->ClassName == className)
            return current;
        current = current->Parent;
    }

    return std::nullopt;
}

std::optional<Instance*> Instance::FindFirstAncestorWhichIsA(std::string& className) {
    Instance* current = Parent;
    while (current) {
        if (current->IsA(className))
            return current;
        current = current->Parent;
    }

    return std::nullopt;
}

std::optional<Instance*> Instance::FindFirstChild(std::string& name) {
    for (auto* child : Children)
        if (child->Name == name)
            return child;

    return std::nullopt;
}

std::optional<Instance*> Instance::FindFirstChildOfClass(std::string& className) {
    for (auto* child : Children)
        if (child->ClassName == className)
            return child;

    return std::nullopt;
}

std::optional<Instance*> Instance::FindFirstChildWhichIsA(std::string& className) {
    for (auto* child : Children)
        if (child->IsA(className))
            return child;

    return std::nullopt;
}

std::optional<Instance*> Instance::FindFirstDescendant(std::string& name) {
    for (auto* child : Children) {
        if (child->Name == name)
            return child;
        auto result = child->FindFirstDescendant(name);
        if (result)
            return result;
    }
    return std::nullopt;
}

std::vector<Instance*> Instance::GetChildren() {
    return Children;
}

std::vector<Instance*> Instance::GetDescendants() {
    std::vector<Instance*> descendants;
    std::vector<Instance*> stack = Children;

    while (!stack.empty()) {
        Instance* current = stack.back();
        stack.pop_back();

        descendants.push_back(current);
        for (auto* child : current->Children)
            stack.push_back(child);
    }

    return descendants;
}

bool Instance::IsAncestorOf(Instance* descendant) {
    if (!descendant) return false;

    Instance* current = descendant->Parent;
    while (current) {
        if (current == this)
            return true;
        current = current->Parent;
    }

    return false;
}

bool Instance::IsDescendantOf(Instance* ancestor) {
    if (!ancestor) return false;
    return ancestor->IsAncestorOf(this);
}

void Instance::ClearAllChildren() {
    for (auto* child : Children) {
        if (child)
            child->Destroy();
    }

    Children.clear();
}

static int Instance_index(lua_State* L) {
    Instance** pinst = (Instance**)luaL_checkudata(L, 1, "Instance");
    Instance* inst = *pinst;
    const char* key = luaL_checkstring(L, 2);

    if (strcmp(key, "Name") == 0) {
        lua_pushstring(L, inst->Name.c_str());
    } else if (strcmp(key, "ClassName") == 0) {
        lua_pushstring(L, inst->ClassName.c_str());
    } else if (strcmp(key, "Parent") == 0) {
        if (inst->Parent) {
            //Return parent based on its type
            if (inst->Parent->ClassName == "Workspace") {
                lua_getglobal(L, "workspace");
            } else if (inst->Parent->ClassName == "DataModel") {
                lua_getglobal(L, "game");
            } else {
                Instance** udata = (Instance**)lua_newuserdata(L, sizeof(Instance*));
                *udata = inst->Parent;
                luaL_getmetatable(L, "Instance");
                lua_setmetatable(L, -2);
            }
        } else {
            lua_pushnil(L);
        }
    } else {
        lua_pushnil(L);
    }

    return 1;
}


static int Instance_newindex(lua_State* L) {
    Instance** pinst = (Instance**)luaL_checkudata(L, 1, "Instance");
    Instance* inst = *pinst;
    const char* key = luaL_checkstring(L, 2);

    if (strcmp(key, "Name") == 0) {
        inst->Name = luaL_checkstring(L, 3);
    } else if (strcmp(key, "Parent") == 0) {
        if (lua_isnil(L, 3)) {
            inst->SetParent(nullptr);
        } else {
            //Try different parent types
            Instance* newParent = nullptr;

            //Check if it's workspace
            if (lua_isuserdata(L, 3)) {
                void* ud = lua_touserdata(L, 3);
                if (ud != nullptr && lua_getmetatable(L, 3)) {
                    luaL_getmetatable(L, "WorkspaceMeta");
                    if (lua_rawequal(L, -1, -2)) {
                        Workspace** pws = (Workspace**)ud;
                        newParent = *pws;
                        lua_pop(L, 2);
                    } else {
                        lua_pop(L, 1);
                        luaL_getmetatable(L, "PartMeta");
                        if (lua_rawequal(L, -1, -2)) {
                            Part** ppart = (Part**)ud;
                            newParent = *ppart;
                            lua_pop(L, 2);
                        } else {
                            lua_pop(L, 1);
                            luaL_getmetatable(L, "Instance");
                            if (lua_rawequal(L, -1, -2)) {
                                Instance** pinst2 = (Instance**)ud;
                                newParent = *pinst2;
                            }
                            lua_pop(L, 2);
                        }
                    }
                } else {
                    lua_pop(L, 1);
                }
            }

            if (newParent) {
                inst->SetParent(newParent);
            }
        }
    }

    return 0;
}

bool Instance::IsA(const std::string& className) const {
    if (className == "Instance") return true;
    return this->ClassName == className || Object::IsA(className);
}

//-- Lua Bindings --//

static bool inst_user_is_meta(lua_State* L, int idx, const char* meta) {
    if (!lua_isuserdata(L, idx)) return false;
    if (!lua_getmetatable(L, idx)) return false;//pushes metatable
    luaL_getmetatable(L, meta);//pushes registered metatable
    bool eq = lua_rawequal(L, -1, -2);
    lua_pop(L, 2);
    return eq;
}

static Instance* get_instance_from_any(lua_State* L, int idx) {
    if (inst_user_is_meta(L, idx, "Instance")) {
        return *(Instance**)lua_touserdata(L, idx);
    }
    if (inst_user_is_meta(L, idx, "BasePartMeta")) {
        return *(BasePart**)lua_touserdata(L, idx);
    }
    if (inst_user_is_meta(L, idx, "PartMeta")) {
        return *(Part**)lua_touserdata(L, idx);
    }
    luaL_typeerrorL(L, idx, "Instance or derived userdata");
    return nullptr;
}

static int l_instance_isA(lua_State* L) {
    Instance* inst = get_instance_from_any(L, 1);
    const char* className = luaL_checkstring(L, 2);
    lua_pushboolean(L, inst->IsA(className));
    return 1;
}

static int l_instance_getChildren(lua_State* L) {
    Instance* inst = get_instance_from_any(L, 1);
    lua_newtable(L);
    int i = 1;
    for (Instance* child : inst->GetChildren()) {
        lua_pushinteger(L, i++);
        Instance** udata = (Instance**)lua_newuserdata(L, sizeof(Instance*));
        *udata = child;
        luaL_getmetatable(L, "Instance");
        lua_setmetatable(L, -2);
        lua_settable(L, -3);
    }
    return 1;
}

static int l_instance_getParent(lua_State* L) {
    Instance* inst = get_instance_from_any(L, 1);
    if (inst->Parent) {
        Instance** udata = (Instance**)lua_newuserdata(L, sizeof(Instance*));
        *udata = inst->Parent;
        luaL_getmetatable(L, "Instance");
        lua_setmetatable(L, -2);
    } else {
        lua_pushnil(L);
    }
    return 1;
}

static int l_instance_getName(lua_State* L) {
    Instance* inst = get_instance_from_any(L, 1);
    lua_pushstring(L, inst->Name.c_str());
    return 1;
}

static int l_instance_setName(lua_State* L) {
    Instance* inst = get_instance_from_any(L, 1);

    const char* name = luaL_checkstring(L, 2);
    if (strcmp(name, "Name") == 0) {
        const char* newName = luaL_checkstring(L, 3);
        inst->Name = newName;
    }

    return 0;
}

static int l_instance_setParent(lua_State* L) {
    Instance* inst = get_instance_from_any(L, 1);
    Instance* parent = get_instance_from_any(L, 2);
    if (parent) inst->SetParent(parent);
    return 0;
}

static int l_instance_index(lua_State* L) {
    (void)get_instance_from_any(L, 1);//validate and allow derived types
    const char* key = luaL_checkstring(L, 2);

    luaL_getmetatable(L, "Instance");
    lua_getfield(L, -1, key);

    if (!lua_isnil(L, -1)) {
        return 1;
    }
    lua_pop(L, 2);

    if (strcmp(key, "Parent") == 0) {
        return l_instance_getParent(L);
    } else if (strcmp(key, "Name") == 0) {
        return l_instance_getName(L);
    }

    return 0;
}

static const luaL_Reg instancelib[] = {
    {"IsA", l_instance_isA},
    {"GetChildren", l_instance_getChildren},
    {NULL, NULL}};

void Class_Instance_Bind(lua_State* L) {
    luaL_newmetatable(L, "Instance");
    luaL_register(L, NULL, instancelib);

    lua_pushcfunction(L, l_instance_index, "__index");
    lua_setfield(L, -2, "__index");

    lua_pushcfunction(L, l_instance_setName, "__newindex");
    lua_setfield(L, -2, "__newindex");

    //Ensure equality compares underlying C++ pointers
    lua_pushcfunction(L, Lua_UserdataPtrEq, "__eq");
    lua_setfield(L, -2, "__eq");

    lua_pop(L, 1);
}