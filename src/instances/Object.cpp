#include "Object.h"

Object::Object(const std::string& className)
    : ClassName(className), Name(className) {
}

Object::~Object() {
}

bool Object::IsA(const std::string& className) const {
    return this->ClassName == className || className == "Object";
}

void Object::FirePropertyChanged(const std::string& propertyName) {
    Changed.Fire(propertyName);
}
