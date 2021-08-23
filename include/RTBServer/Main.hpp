#pragma once

#include "RollTheBall/Main.hpp"
#include "Wasabi/Core/WTimer.hpp"

#include "WasabiGame/Utilities/Config.hpp"
#include "WasabiGame/Utilities/Scheduler.hpp"
#include "WasabiGame/Networking/NetworkManager.hpp"
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

	class ServerApplication : public std::enable_shared_from_this<ServerApplication> {
		
		/* 
		 * cells are logical partitions of players (e.g. game rooms, battlegrounds, separate maps, etc...)
		 * m_cells[0] is the login cell and m_cells[1] is the matchmaking cell, both are created on startup
		 */
		std::vector<std::shared_ptr<ServerCell>> m_cells;
		std::vector<std::shared_ptr<ServerCell>> m_uninitializedCells;
		std::vector<std::shared_ptr<ServerCell>> m_deadCells;
		std::mutex m_cellsMutex;
		std::mutex m_uninitializedCellsMutex;
		std::mutex m_deadCellsMutex;

		void UpdateCellsThread();
		void InitializeCellsThread();
		void CleanupCellsThread();

	public:
		ServerApplication(bool generateAssets = true);
		~ServerApplication();

		void Run();

		WTimer Timer;
		std::shared_ptr<ServerClientsRepository> ClientsRepository;
		std::shared_ptr<WasabiGame::GameScheduler> Scheduler;
		std::shared_ptr<WasabiGame::GameConfig> Config;
		std::shared_ptr<WasabiGame::NetworkManager> Networking;

		void RegisterCell(std::shared_ptr<ServerCell> cell);
		std::shared_ptr<ServerCell> GetLoginCell() const;
		std::shared_ptr<ServerCell> GetMatchmakingCell() const;
	};

};
