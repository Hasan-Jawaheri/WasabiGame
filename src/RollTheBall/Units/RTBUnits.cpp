#include "RollTheBall/Units/RTBUnits.hpp"
#include "WasabiGame/Units/Units.hpp"

#include "RollTheBall/Units/Player.hpp"
#include "RollTheBall/Units/TestUnitAI.hpp"

void SetupRTBUnits(UnitsManager* manager) {
	manager->ResetUnits();
	manager->RegisterUnit(UNIT_PLAYER, [manager]() { return manager->CreateUnitAndAI<Player, PlayerAI>(std::string(Player::modelName)); });
	manager->RegisterUnit(UNIT_TEST_BALL, [manager]() { return manager->CreateUnitAndAI<BallUnit, TestUnitAI>(std::string(BallUnit::modelName)); });
}
