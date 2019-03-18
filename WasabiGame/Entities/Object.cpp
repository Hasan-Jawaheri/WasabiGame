#include "Object.hpp"

Object::Object(UINT ID, TYPE t) {
	__ID = ID;
	obj = nullptr;
	type = t;
}

Object::~Object() {
	Destroy();
}

WOrientation* Object::GetOrientation() const {
	return (WOrientation*)obj;
}
Object::TYPE Object::GetType() const {
	return type;
}

void Object::Load() {
	obj = new WObject(APPHANDLE);
}
bool Object::Update(float fDeltaTime) {
	return true;
}

void Object::Destroy() {
	if (obj) {
		W_SAFE_REMOVEREF(obj);
	}
}

