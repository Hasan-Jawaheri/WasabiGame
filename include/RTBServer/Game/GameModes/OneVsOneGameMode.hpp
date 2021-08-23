#pragma once

#include "RTBServer/Game/GameModes/GameMode.hpp"
#include "RTBServer/Simulation/Simulation.hpp"


namespace RTBServer {

	class OneVsOneGameMode : public GameMode {
		std::shared_ptr<ServerSimulation> m_simulation;
		std::unordered_map<uint32_t, std::shared_ptr<RTBConnectedPlayer>> m_connectedPlayers;

	public:
		static GAME_MODE_MATCHMAKING_PROPERTIES MATCHMAKING_PROPERTIES;

		OneVsOneGameMode(std::weak_ptr<ServerApplication> server);
		virtual ~OneVsOneGameMode();

		virtual void Initialize(std::weak_ptr<ServerCell> cell) override;
		virtual void Update() override;
		virtual void Cleanup() override;
		virtual void OnClientAdded(std::shared_ptr<ServerConnectedClient> client) override;
		virtual void OnClientRemoved(std::shared_ptr<ServerConnectedClient> client) override;
	};
}
