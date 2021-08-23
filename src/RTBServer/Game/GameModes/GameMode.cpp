#include "RTBServer/Game/GameModes/GameMode.hpp"
#include "RTBServer/Game/GameModes/OneVsOneGameMode.hpp"


RTBServer::GameMode::GameMode(std::weak_ptr<ServerApplication> server): std::enable_shared_from_this<GameMode>() {
	m_server = server;
}

RTBServer::GameMode::~GameMode() {
}

RTBServer::GAME_MODE_MATCHMAKING_PROPERTIES RTBServer::GetGameModeMatchmakingProperties(GAME_MODE gameMode) {
	if (gameMode == GAME_MODE::GAME_MODE_ONE_VS_ONE)
		return OneVsOneGameMode::MATCHMAKING_PROPERTIES;
	else // by default use 1v1 mode
		return OneVsOneGameMode::MATCHMAKING_PROPERTIES;
}

std::shared_ptr<RTBServer::GameMode> RTBServer::CreateGameMode(GAME_MODE gameMode, std::weak_ptr<ServerApplication> server) {
	if (gameMode == GAME_MODE::GAME_MODE_ONE_VS_ONE)
		return std::make_shared<OneVsOneGameMode>(server);
	else // by default use 1v1 mode
		return std::make_shared<OneVsOneGameMode>(server);
}
