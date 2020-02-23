#pragma once

#include "WasabiGame/Utilities/Config.hpp"
#include "WasabiGame/Utilities/Scheduler.hpp"
#include "RTBServer/Simulation/Simulation.hpp"
#include "RTBServer/Networking/Networking.hpp"
#include "RTBServer/Game/RTBConnectedPlayer.hpp"

#include <cstdint>
#include <unordered_map>


namespace RTBServer {

	class ServerNetworking;
	class ServerConnectedClient;
	class ServerSimulation;

	class ServerApplication : public std::enable_shared_from_this<ServerApplication> {
		std::unordered_map<uint32_t, std::shared_ptr<RTBConnectedPlayer>> m_connectedPlayers;

	public:
		ServerApplication();

		std::shared_ptr<WasabiGame::GameConfig> Config;
		std::shared_ptr<WasabiGame::GameScheduler> Scheduler;
		std::shared_ptr<ServerNetworking> Networking;
		std::shared_ptr<ServerSimulation> Simulation;

		void Initialize(bool generateAssets = true);
		void Destroy();
		void Run();

		void OnClientConnected(std::shared_ptr<ServerConnectedClient> client);
		void OnClientDisconnected(std::shared_ptr<ServerConnectedClient> client);
	};

};
