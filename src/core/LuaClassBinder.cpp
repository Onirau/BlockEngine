#include "LuaClassBinder.h"
#include "../instances/Instance.h"
#include "../instances/BasePart.h"
#include "../instances/Part.h"
#include "../datatypes/Vector3.h"
#include "../datatypes/Color3.h"
#include <unordered_set>

std::unordered_map<std::string, ClassDescriptor> LuaClassBinder::s_classes;

void LuaClassBinder::RegisterClass(const std::string& className,
                                   const std::string& parentClassName) {
    ClassDescriptor desc;
    desc.className = className;
    desc.parentClassName = parentClassName;
    s_classes[className] = desc;
}

void LuaClassBinder::AddProperty(const std::string& className,
                                 const std::string& propName,
                                 PropertyGetter getter,
                                 PropertySetter setter) {
    auto it = s_classes.find(className);
    if (it != s_classes.end()) {
        PropertyDescriptor prop;
        prop.getter = getter;
        prop.setter = setter;
        prop.readonly = (setter == nullptr);
        it->second.properties[propName] = prop;
    }
}

void LuaClassBinder::AddMethod(const std::string& className,
                               const std::string& methodName,
                               MethodFunc method) {
    auto it = s_classes.find(className);
    if (it != s_classes.end()) {
        it->second.methods[methodName] = method;
    }
}

void LuaClassBinder::SetConstructor(const std::string& className,
                                    std::function<Instance*()> ctor) {
    auto it = s_classes.find(className);
    if (it != s_classes.end()) {
        it->second.constructor = ctor;
    }
}

bool LuaClassBinder::IsA(lua_State* L, int idx, const std::string& className) {
    if (!lua_isuserdata(L, idx)) return false;

    Instance** pinst = (Instance**)lua_touserdata(L, idx);
    if (!pinst || !*pinst) return false;

    return (*pinst)->IsA(className);
}

Instance* LuaClassBinder::CheckInstance(lua_State* L, int idx, const std::string& className) {
    Instance** pinst = (Instance**)lua_touserdata(L, idx);
    if (!pinst || !*pinst) {
        luaL_error(L, "Invalid instance pointer");
        return nullptr;
    }

    if (!className.empty() && !(*pinst)->IsA(className)) {
        luaL_error(L, "Expected %s, got %s", className.c_str(), (*pinst)->ClassName.c_str());
        return nullptr;
    }

    return *pinst;
}

void LuaClassBinder::PushInstance(lua_State* L, Instance* inst) {
    if (!inst) {
        lua_pushnil(L);
        return;
    }

    Instance** udata = (Instance**)lua_newuserdata(L, sizeof(Instance*));
    *udata = inst;

    std::string metaName = GetMetatableName(inst->ClassName);
    luaL_getmetatable(L, metaName.c_str());
    lua_setmetatable(L, -2);
}

int LuaClassBinder::MethodClosure(lua_State* L) {
    //Get instance from upvalue (light userdata)
    Instance* inst = (Instance*)lua_tolightuserdata(L, lua_upvalueindex(1));

    //Get method name from upvalue
    const char* methodName = lua_tostring(L, lua_upvalueindex(2));

    if (!inst) {
        luaL_error(L, "Invalid instance in method call");
    }

    //Find the method in the class hierarchy
    std::string currentClass = inst->ClassName;
    while (!currentClass.empty()) {
        auto* desc = GetDescriptor(currentClass);
        if (desc) {
            auto it = desc->methods.find(methodName);
            if (it != desc->methods.end()) {
                return it->second(L, inst);
            }
            currentClass = desc->parentClassName;
        } else {
            break;
        }
    }

    luaL_error(L, "Method '%s' not found on class '%s'",
               methodName, inst->ClassName.c_str());
}

int LuaClassBinder::GenericIndex(lua_State* L) {
    Instance* inst = CheckInstance(L, 1);
    const char* key = luaL_checkstring(L, 2);

    //todo: add some kind of lookup

    printf("GenericIndex called: object class='%s', looking for key='%s'\n",
           inst->ClassName.c_str(), key);

    //Walk up the inheritance chain
    std::string currentClass = inst->ClassName;
    std::vector<std::string> checked;

    while (!currentClass.empty()) {
        checked.push_back(currentClass);
        auto* desc = GetDescriptor(currentClass);

        if (!desc) {
            printf("  WARNING: No descriptor found for class '%s'\n", currentClass.c_str());
            break;
        }

        printf("  Checking class '%s' (has %zu properties, %zu methods)\n",
               currentClass.c_str(), desc->properties.size(), desc->methods.size());

        //Check properties FIRST
        auto propIt = desc->properties.find(key);
        if (propIt != desc->properties.end()) {
            printf("  FOUND property '%s' in class '%s'\n", key, currentClass.c_str());
            if (propIt->second.getter) {
                return propIt->second.getter(L, inst);
            } else {
                printf("  WARNING: Property '%s' found but has no getter\n", key);
            }
        }

        //Check methods
        auto methodIt = desc->methods.find(key);
        if (methodIt != desc->methods.end()) {
            printf("  FOUND method '%s' in class '%s'\n", key, currentClass.c_str());
            //Push instance as light userdata
            lua_pushlightuserdata(L, inst);
            //Store method name
            lua_pushstring(L, key);
            //Create closure that will call the method
            lua_pushcclosure(L, MethodClosure, "method", 2);
            return 1;
        }

        currentClass = desc->parentClassName;
        printf("  Not found in '%s', moving to parent '%s'\n",
               checked.back().c_str(), currentClass.c_str());
    }

    printf("  Property/method '%s' not found in hierarchy: ", key);
    for (const auto& c : checked)
        printf("%s ", c.c_str());
    printf("\n");

    lua_pushnil(L);
    return 1;
}

int LuaClassBinder::GenericNewIndex(lua_State* L) {
    Instance* inst = CheckInstance(L, 1);
    const char* key = luaL_checkstring(L, 2);

    //Walk up the inheritance chain
    std::string currentClass = inst->ClassName;
    while (!currentClass.empty()) {
        auto* desc = GetDescriptor(currentClass);
        if (!desc) break;

        auto propIt = desc->properties.find(key);
        if (propIt != desc->properties.end()) {
            if (propIt->second.readonly) {
                luaL_error(L, "Property '%s' is readonly", key);
                return 0;
            }
            if (propIt->second.setter) {
                return propIt->second.setter(L, inst, 3);
            }
        }

        currentClass = desc->parentClassName;
    }

    luaL_error(L, "Unknown property '%s'", key);
    return 0;
}

int LuaClassBinder::GenericToString(lua_State* L) {
    Instance* inst = CheckInstance(L, 1);
    lua_pushfstring(L, "%s: %s", inst->ClassName.c_str(), inst->Name.c_str());
    return 1;
}

int LuaClassBinder::GenericEq(lua_State* L) {
    Instance* a = CheckInstance(L, 1);
    Instance* b = CheckInstance(L, 2);
    lua_pushboolean(L, a == b);
    return 1;
}

int LuaClassBinder::GenericGC(lua_State* L) {
    //Don't delete instances here - they're managed elsewhere
    return 0;
}

int LuaClassBinder::GenericConstructor(lua_State* L) {
    const char* className = luaL_checkstring(L, 1);

    auto* desc = GetDescriptor(className);
    if (!desc || !desc->constructor) {
        luaL_error(L, "Cannot create instance of '%s'", className);
        return 0;
    }

    Instance* inst = desc->constructor();
    PushInstance(L, inst);
    return 1;
}

ClassDescriptor* LuaClassBinder::GetDescriptor(const std::string& className) {
    auto it = s_classes.find(className);
    return (it != s_classes.end()) ? &it->second : nullptr;
}

std::string LuaClassBinder::GetMetatableName(const std::string& className) {
    return className + "Meta";
}

void LuaClassBinder::CreateMetatable(lua_State* L, const std::string& className) {
    std::string metaName = GetMetatableName(className);
    printf("  CreateMetatable: Creating '%s' for class '%s'\n",
           metaName.c_str(), className.c_str());

    luaL_newmetatable(L, metaName.c_str());

    lua_pushcfunction(L, GenericIndex, "__index");
    lua_setfield(L, -2, "__index");

    lua_pushcfunction(L, GenericNewIndex, "__newindex");
    lua_setfield(L, -2, "__newindex");

    lua_pushcfunction(L, GenericToString, "__tostring");
    lua_setfield(L, -2, "__tostring");

    lua_pushcfunction(L, GenericEq, "__eq");
    lua_setfield(L, -2, "__eq");

    lua_pushcfunction(L, GenericGC, "__gc");
    lua_setfield(L, -2, "__gc");

    //Set parent metatable for inheritance
    auto* desc = GetDescriptor(className);
    if (desc && !desc->parentClassName.empty()) {
        std::string parentMeta = GetMetatableName(desc->parentClassName);
        printf("    Setting parent metatable '%s'\n", parentMeta.c_str());
        luaL_getmetatable(L, parentMeta.c_str());
        if (!lua_isnil(L, -1)) {
            lua_setmetatable(L, -2);
            printf("    Parent metatable set successfully\n");
        } else {
            printf("    WARNING: Parent metatable '%s' not found!\n", parentMeta.c_str());
            lua_pop(L, 1);//Pop the nil
        }
    }

    lua_pop(L, 1);
}

void LuaClassBinder::BindAll(lua_State* L) {
    printf("=== LuaClassBinder::BindAll starting ===\n");
    printf("Total classes registered: %zu\n", s_classes.size());

    //Print all registered classes first
    for (const auto& [className, desc] : s_classes) {
        printf("  Class '%s' -> parent '%s' (%zu props, %zu methods)\n",
               className.c_str(),
               desc.parentClassName.c_str(),
               desc.properties.size(),
               desc.methods.size());
    }

    //Create metatables in order (parents before children)
    std::unordered_set<std::string> created;
    bool progress = true;

    while (progress && created.size() < s_classes.size()) {
        progress = false;
        for (const auto& [className, desc] : s_classes) {
            if (created.count(className)) continue;

            //Check if parent is created (or no parent)
            if (desc.parentClassName.empty() || created.count(desc.parentClassName)) {
                printf("Creating metatable for '%s'\n", className.c_str());
                CreateMetatable(L, className);
                created.insert(className);
                progress = true;
            }
        }
    }

    if (created.size() < s_classes.size()) {
        printf("WARNING: Not all classes were bound! Created %zu of %zu\n",
               created.size(), s_classes.size());
        for (const auto& [className, desc] : s_classes) {
            if (!created.count(className)) {
                printf("  MISSING: %s (parent: %s)\n",
                       className.c_str(), desc.parentClassName.c_str());
            }
        }
    }

    //Create Instance.new()
    printf("Creating Instance.new() constructor\n");
    lua_newtable(L);
    lua_pushcfunction(L, GenericConstructor, "new");
    lua_setfield(L, -2, "new");
    lua_setglobal(L, "Instance");

    printf("=== LuaClassBinder::BindAll complete ===\n");
}