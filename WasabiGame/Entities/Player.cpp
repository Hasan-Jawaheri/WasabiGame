#include "Player.hpp"
#include "../AI/AI.hpp"

Character::Character(UINT ID) : Unit(ID, ID) {
}

Character::~Character() {
}

Player::Player() : Unit(0, 0) {
	__ID = 0;
}

Player::~Player() {
	Destroy();
}

void Player::Load() {
	//for now, just load it as default
	Unit::Load();

	//load an equipment as a test
	Equip(SLOT_WEAPON1, 2);
	//Equip ( SLOT_HEAD, 3 );
	Equip(SLOT_PANTS, 4);
	Equip(SLOT_SHOULDERS, 5);
	Equip(SLOT_BELT, 6);
	Equip(SLOT_BOOTS, 7);
}

void Player::Destroy() {
	Unit::Destroy();
}

bool Player::Update(float fDeltaTime) {
	Unit::Update(fDeltaTime);

	return true; //never delete the player
}

void Player::Equip(ITEMSLOT slot, UINT itemID) {
	Unit::Equip(slot, itemID);
}

void Player::Unequip(ITEMSLOT slot) {
	Unit::Unequip(slot);
}

