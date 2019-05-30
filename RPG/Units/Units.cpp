#include "RPGUnits.hpp"
#include "../../WasabiGame/Units/UnitsManager.hpp"
#include "../../WasabiGame/Units/Units.hpp"



void SetupRPGUnits() {
	MapLoader::ResetMaps();
	MapLoader::RegisterMap(UNIT_PLAYER, UnitsManager::CreateUnitAndAI<Player, PlayerAI>(""));
}
