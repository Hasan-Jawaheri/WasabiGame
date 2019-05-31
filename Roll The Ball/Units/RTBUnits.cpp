#include "RTBUnits.hpp"
#include "../../WasabiGame/Units/Units.hpp"

#include "Player.hpp"

void SetupRTBUnits() {
	UnitsManager::ResetUnits();
	UnitsManager::RegisterUnit(UNIT_PLAYER, []() { return UnitsManager::CreateUnitAndAI<Player, PlayerAI>(std::string(Player::modelName)); });
}
