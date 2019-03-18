#pragma once

#include "../Common.hpp"
#include "Unit.hpp"

class Character : public Unit {
public:
	Character(UINT ID = 0);
	virtual ~Character();
};

class Player : public Unit {
public:
	Player();
	virtual ~Player();

	virtual void Load();
	virtual void Destroy();
	virtual bool Update(float fDeltaTime);

	virtual void Equip(ITEMSLOT slot, UINT itemID);
	virtual void Unequip(ITEMSLOT slot);
};

