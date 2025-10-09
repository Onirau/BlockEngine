#include "Instance.h"
#include "../core/LuaBindings.h"
#include "../core/LuaClassBinder.h"
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

//------ Hierarchy ------//

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

bool Instance::IsA(const std::string& className) const {
    if (className == "Instance") return true;
    return this->ClassName == className || Object::IsA(className);
}

//Bind

void Class_Instance_Bind(lua_State* L) {
    LuaClassBinder::RegisterClass("Instance", "Object");

    //Properties
    LuaClassBinder::AddProperty("Instance", "Name", [](lua_State* L, Instance* inst) -> int {
            lua_pushstring(L, inst->Name.c_str());
            return 1; }, [](lua_State* L, Instance* inst, int valueIdx) -> int {
            inst->Name = luaL_checkstring(L, valueIdx);
            return 0; });

    LuaClassBinder::AddProperty("Instance", "ClassName", [](lua_State* L, Instance* inst) -> int {
            lua_pushstring(L, inst->ClassName.c_str());
            return 1; },
                                nullptr);//Read-only

    LuaClassBinder::AddProperty("Instance", "Parent", [](lua_State* L, Instance* inst) -> int {
            if (inst->Parent) {
                LuaClassBinder::PushInstance(L, inst->Parent);
            } else {
                lua_pushnil(L);
            }
            return 1; }, [](lua_State* L, Instance* inst, int valueIdx) -> int {
            if (lua_isnil(L, valueIdx)) {
                inst->SetParent(nullptr);
            } else {
                Instance* newParent = LuaClassBinder::CheckInstance(L, valueIdx);
                inst->SetParent(newParent);
            }
            return 0; });

    //Methods
    LuaClassBinder::AddMethod("Instance", "IsA",
                              [](lua_State* L, Instance* inst) -> int {
                                  const char* className = luaL_checkstring(L, 2);
                                  lua_pushboolean(L, inst->IsA(className));
                                  return 1;
                              });

    LuaClassBinder::AddMethod("Instance", "GetChildren",
                              [](lua_State* L, Instance* inst) -> int {
                                  lua_newtable(L);
                                  int i = 1;
                                  for (Instance* child : inst->GetChildren()) {
                                      lua_pushinteger(L, i++);
                                      LuaClassBinder::PushInstance(L, child);
                                      lua_settable(L, -3);
                                  }
                                  return 1;
                              });

    LuaClassBinder::AddMethod("Instance", "GetDescendants",
                              [](lua_State* L, Instance* inst) -> int {
                                  lua_newtable(L);
                                  int i = 1;
                                  for (Instance* descendant : inst->GetDescendants()) {
                                      lua_pushinteger(L, i++);
                                      LuaClassBinder::PushInstance(L, descendant);
                                      lua_settable(L, -3);
                                  }
                                  return 1;
                              });

    LuaClassBinder::AddMethod("Instance", "FindFirstChild",
                              [](lua_State* L, Instance* inst) -> int {
                                  std::string name = luaL_checkstring(L, 2);
                                  auto result = inst->FindFirstChild(name);
                                  if (result.has_value()) {
                                      LuaClassBinder::PushInstance(L, result.value());
                                  } else {
                                      lua_pushnil(L);
                                  }
                                  return 1;
                              });

    LuaClassBinder::AddMethod("Instance", "FindFirstChildWhichIsA",
                              [](lua_State* L, Instance* inst) -> int {
                                  std::string className = luaL_checkstring(L, 2);
                                  auto result = inst->FindFirstChildWhichIsA(className);
                                  if (result.has_value()) {
                                      LuaClassBinder::PushInstance(L, result.value());
                                  } else {
                                      lua_pushnil(L);
                                  }
                                  return 1;
                              });

    LuaClassBinder::AddMethod("Instance", "Destroy",
                              [](lua_State* L, Instance* inst) -> int {
                                  inst->Destroy();
                                  return 0;
                              });

    LuaClassBinder::AddMethod("Instance", "ClearAllChildren",
                              [](lua_State* L, Instance* inst) -> int {
                                  inst->ClearAllChildren();
                                  return 0;
                              });

    LuaClassBinder::AddMethod("Instance", "IsAncestorOf",
                              [](lua_State* L, Instance* inst) -> int {
                                  Instance* descendant = LuaClassBinder::CheckInstance(L, 2);
                                  lua_pushboolean(L, inst->IsAncestorOf(descendant));
                                  return 1;
                              });

    LuaClassBinder::AddMethod("Instance", "IsDescendantOf",
                              [](lua_State* L, Instance* inst) -> int {
                                  Instance* ancestor = LuaClassBinder::CheckInstance(L, 2);
                                  lua_pushboolean(L, inst->IsDescendantOf(ancestor));
                                  return 1;
                              });
}