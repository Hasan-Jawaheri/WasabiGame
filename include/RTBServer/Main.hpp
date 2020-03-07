#pragma once

#include "RollTheBall/Main.hpp"

#include "WasabiGame/Utilities/Config.hpp"
#include "WasabiGame/Utilities/Scheduler.hpp"
#include "RTBServer/Networking/Networking.hpp"
#include "RTBServer/Game/RTBConnectedPlayer.hpp"

#include <cstdint>
#include <unordered_map>


namespace RTBServer {

	class ServerConnectedClient;
	class ServerSimulationGameState;

	class ServerApplication : public WasabiGame::WasabiBaseGame {
		std::unordered_map<uint32_t, std::shared_ptr<RTBConnectedPlayer>> m_connectedPlayers;

	public:
		ServerApplication(bool generateAssets = true, bool enableVulkanDebugging = true, bool enablePhysicsDebugging = true);
		~ServerApplication();

		std::shared_ptr<ServerSimulationGameState> Simulation;
		void SwitchToInitialState();

		void OnClientConnected(std::shared_ptr<ServerConnectedClient> client);
		void OnClientDisconnected(std::shared_ptr<ServerConnectedClient> client);
	};

};
