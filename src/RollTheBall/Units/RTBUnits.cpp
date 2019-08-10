#include "RollTheBall/Units/RTBUnits.hpp"
#include "WasabiGame/Units/Units.hpp"

#include "RollTheBall/Units/Player.hpp"

void SetupRTBUnits() {
	UnitsManager::ResetUnits();
	UnitsManager::RegisterUnit(UNIT_PLAYER, []() { return UnitsManager::CreateUnitAndAI<Player, PlayerAI>(std::string(Player::modelName)); });
}
