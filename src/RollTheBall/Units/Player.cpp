#include "RollTheBall/Units/Player.hpp"
#include "WasabiGame/Units/UnitsManager.hpp"

const char* Player::modelName = "player";

Player::Player(Wasabi* app, ResourceManager* resourceManager, UnitsManager* unitsManager) : BallUnit(app, resourceManager, unitsManager) {
}

Player::~Player() {
}

void Player::Update(float fDeltaTime) {
	BallUnit::Update(fDeltaTime);
}
