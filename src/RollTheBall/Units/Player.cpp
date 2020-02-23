#include "RollTheBall/Units/Player.hpp"
#include "WasabiGame/Units/UnitsManager.hpp"


const char* RollTheBall::Player::modelName = "player";

RollTheBall::Player::Player(std::shared_ptr<WasabiGame::WasabiBaseGame> app, std::shared_ptr<class WasabiGame::ResourceManager> resourceManager, std::shared_ptr<class WasabiGame::UnitsManager> unitsManager) : BallUnit(app, resourceManager, unitsManager) {
}

RollTheBall::Player::~Player() {
}

void RollTheBall::Player::Update(float fDeltaTime) {
	BallUnit::Update(fDeltaTime);
}
