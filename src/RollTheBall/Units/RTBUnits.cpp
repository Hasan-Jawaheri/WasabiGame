#include "RollTheBall/Units/RTBUnits.hpp"
#include "WasabiGame/Units/Units.hpp"

#include "RollTheBall/AI/PlayerAI.hpp"
#include "RollTheBall/AI/RemoteControlledAI.hpp"
#include "RollTheBall/AI/TestUnitAI.hpp"

#include "RollTheBall/Units/Player.hpp"
#include "RollTheBall/Units/BallUnit.hpp"


template<typename UNIT, typename CLIENT_AI, typename SERVER_AI>
static std::function<std::shared_ptr<WasabiGame::Unit>()> GenerateUnit(std::shared_ptr<WasabiGame::UnitsManager> manager, bool isServer, std::string modelName) {
	return [manager, isServer, modelName]() {
		if (isServer)
			return std::static_pointer_cast<WasabiGame::Unit>(manager->CreateUnitAndAI<UNIT, SERVER_AI>(modelName));
		else
			return std::static_pointer_cast<WasabiGame::Unit>(manager->CreateUnitAndAI<UNIT, CLIENT_AI>(modelName));
	};
}

void RollTheBall::SetupRTBUnits(std::shared_ptr<WasabiGame::UnitsManager> manager, bool isServer) {
	manager->ResetUnits();
	manager->RegisterUnit(UNIT_PLAYER, GenerateUnit<Player, PlayerAI, RemoteControlledAI>(manager, isServer, Player::modelName));
	manager->RegisterUnit(UNIT_OTHER_PLAYER, GenerateUnit<Player, RemoteControlledAI, RemoteControlledAI>(manager, isServer, Player::modelName));
	manager->RegisterUnit(UNIT_TEST_BALL, GenerateUnit<BallUnit, RemoteControlledAI, TestUnitAI>(manager, isServer, BallUnit::modelName));
}
