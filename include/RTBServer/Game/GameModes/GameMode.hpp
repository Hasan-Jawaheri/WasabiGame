#pragma once
#include "RTBServer/Main.hpp"


namespace RTBServer {

	struct GAME_MODE_MATCHMAKING_PROPERTIES {
		uint32_t minPlayers;
		uint32_t maxPlayers;
	};

	enum class GAME_MODE : uint8_t {
		GAME_MODE_ONE_VS_ONE = 0,
	};

	class GameMode : public std::enable_shared_from_this<GameMode> {
	protected:
		std::weak_ptr<ServerApplication> m_server;

	public:
		GameMode(std::weak_ptr<ServerApplication> server);
		virtual ~GameMode();

		virtual void Initialize(std::weak_ptr<ServerCell> cell) {}
		virtual void Update() {}
		virtual void Cleanup() {}

		virtual void OnClientAdded(std::shared_ptr<ServerConnectedClient> client) {}
		virtual void OnClientRemoved(std::shared_ptr<ServerConnectedClient> client) {}
		virtual bool OnReceivedNetworkUpdate(std::shared_ptr<ServerConnectedClient> client, WasabiGame::NetworkUpdate update) { return false; }
	};
	
	GAME_MODE_MATCHMAKING_PROPERTIES GetGameModeMatchmakingProperties(GAME_MODE gameMode);
	std::shared_ptr<GameMode> CreateGameMode(GAME_MODE gameMode, std::weak_ptr<ServerApplication> server);
}
