#include "RollTheBall/Units/RTBUnits.hpp"
#include "WasabiGame/Units/Units.hpp"

#include "RollTheBall/Units/Player.hpp"
#include "RollTheBall/Units/TestUnitAI.hpp"

void SetupRTBUnits() {
	UnitsManager::ResetUnits();
	UnitsManager::RegisterUnit(UNIT_PLAYER, []() { return UnitsManager::CreateUnitAndAI<Player, PlayerAI>(std::string(Player::modelName)); });
	UnitsManager::RegisterUnit(UNIT_TEST_BALL, []() { return UnitsManager::CreateUnitAndAI<BallUnit, TestUnitAI>(std::string(BallUnit::modelName)); });
}
