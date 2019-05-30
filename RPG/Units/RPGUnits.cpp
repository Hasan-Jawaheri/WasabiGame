#include "RPGUnits.hpp"
#include "../../WasabiGame/Units/Units.hpp"
#include "../../WasabiGame/Units/UnitsManager.hpp"

#include "Player.hpp"
#include "AI/PlayerAI.hpp"

void SetupRPGUnits() {
	UnitsManager::ResetUnits();
	UnitsManager::RegisterUnit(UNIT_PLAYER, []() { return UnitsManager::CreateUnitAndAI<Player, PlayerAI>(std::string(Player::modelName)); });
}
