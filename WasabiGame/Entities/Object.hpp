#pragma once

#include "..//Common.hpp"

enum ITEMSLOT {
	SLOT_HEAD = 0,
	SLOT_NECK = 1,
	SLOT_SHOULDERS = 2,
	SLOT_CHEST = 3,
	SLOT_BRACERS = 4,
	SLOT_GLOVES = 5,
	SLOT_BELT = 6,
	SLOT_PANTS = 7,
	SLOT_BOOTS = 8,
	SLOT_RING1 = 9,
	SLOT_RING2 = 10,
	SLOT_TRINKET1 = 11,
	SLOT_TRINKET2 = 12,
	SLOT_WEAPON1 = 13,
	SLOT_WEAPON2 = 14,
	SLOT_RANGED = 15,
	SLOT_SPECIAL = 16
};
#define NUM_ITEM_SLOTS 16

class Object {
	friend class ResourceManager;
	friend class Unit;

protected:
	UINT __ID;
	WObject* obj;

	enum TYPE {
		TYPE_NONE,			// default
		TYPE_STATIC,		// static (decoration)
		TYPE_DYNAMIC,		// dynamic environment (e.g. elevators)
		TYPE_UNIT,			// a unit (creature/player/boss)
		TYPE_EQUIPMENT
	}	// equipment (sword, chestplate, ...)
	type;

public:
	Object(UINT ID, TYPE t); //id = 0, t = TYPE_NONE
	virtual ~Object();

	virtual void Load();
	virtual void Destroy();
	virtual bool Update(float fDeltaTime);

	virtual WOrientation* GetOrientation() const;
	TYPE GetType() const;
};


