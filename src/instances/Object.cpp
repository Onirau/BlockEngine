#include "Object.h"
#include "../core/LuaClassBinder.h"

//Constructor
Object::Object(const std::string& className)
    : ClassName(className), Name(className) {
}

//Destructor
Object::~Object() {
}

//IsA implementation
bool Object::IsA(const std::string& className) const {
    return this->ClassName == className || className == "Object";
}

//FirePropertyChanged implementation
void Object::FirePropertyChanged(const std::string& propertyName) {
    Changed.Fire(propertyName);
}

//Lua Binding
void Object_Bind(lua_State* L) {
    //Register Object with NO parent (it's the root of the hierarchy)
    LuaClassBinder::RegisterClass("Object", "");

    //ClassName property (read-only)
    LuaClassBinder::AddProperty("Object", "ClassName", [](lua_State* L, Instance* inst) -> int {
            Object* obj = reinterpret_cast<Object*>(inst);
            lua_pushstring(L, obj->ClassName.c_str());
            return 1; }, nullptr);

    //Name property
    LuaClassBinder::AddProperty("Object", "Name", [](lua_State* L, Instance* inst) -> int {
            Object* obj = reinterpret_cast<Object*>(inst);
            lua_pushstring(L, obj->Name.c_str());
            return 1; }, [](lua_State* L, Instance* inst, int valueIdx) -> int {
            Object* obj = reinterpret_cast<Object*>(inst);
            obj->Name = luaL_checkstring(L, valueIdx);
            return 0; });

    //IsA method
    LuaClassBinder::AddMethod("Object", "IsA",
                              [](lua_State* L, Instance* inst) -> int {
                                  Object* obj = reinterpret_cast<Object*>(inst);
                                  const char* className = luaL_checkstring(L, 2);
                                  lua_pushboolean(L, obj->IsA(className));
                                  return 1;
                              });
}