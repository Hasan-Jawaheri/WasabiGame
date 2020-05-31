#pragma once

#include "RollTheBall/Main.hpp"

#include "WasabiGame/Utilities/Config.hpp"
#include "WasabiGame/Utilities/Scheduler.hpp"
#include "RTBServer/Networking/Networking.hpp"
#include "RTBServer/Cells/ServerCell.hpp"
#include "RTBServer/Game/RTBConnectedPlayer.hpp"

#include <cstdint>
#include <unordered_map>


namespace RTBServer {

	class ServerConnectedClient;
	class ServerSimulationGameState;
	class ServerCell;
	class ServerClientsRepository;

	class ServerApplication : public WasabiGame::WasabiBaseGame {
		std::unordered_map<uint32_t, std::shared_ptr<RTBConnectedPlayer>> m_connectedPlayers;
		/* 
		 * cells are logical partitions of players (e.g. game rooms, battlegrounds, separate maps, etc...)
		 * m_cells[0] is the login cell and m_cells[1] is the matchmaking cell, both are created on startup
		 */
		std::vector<std::shared_ptr<ServerCell>> m_cells;

		void UpdateCellsThread();

	public:
		ServerApplication(bool generateAssets = true, bool enableVulkanDebugging = true, bool enablePhysicsDebugging = true);
		~ServerApplication();

		std::shared_ptr<ServerSimulationGameState> Simulation;
		std::shared_ptr<ServerClientsRepository> ClientsRepository;

		void SwitchToInitialState();

		std::shared_ptr<ServerCell> GetLoginCell() const;
		std::shared_ptr<ServerCell> GetMatchmakingCell() const;
	};

};
