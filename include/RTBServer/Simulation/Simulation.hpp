#pragma once

#include "WasabiGame/Utilities/Scheduler.hpp"
#include "RTBServer/Main.hpp"
#include "RTBServer/Game/RTBConnectedPlayer.hpp"

#include <atomic>


class SimulationWasabi;
class SimulationGameState;

namespace RTBServer {

	class ServerApplication;

	class ServerSimulation : public WasabiGame::SchedulerThread, public std::enable_shared_from_this<ServerSimulation> {
		friend class SimulationWasabi;
		friend class SimulationGameState;

		std::atomic<void*> m_gameState;
		bool m_simulationLoaded;
		std::shared_ptr<ServerApplication> m_server;
		bool m_generateAssets;

		void WaitForSimulationLaunch();

	public:
		ServerSimulation(std::shared_ptr<ServerApplication> server, bool generateAssets = true);

		void Run();

		void AddPlayer(std::shared_ptr<RTBConnectedPlayer> player);
		void RemovePlayer(std::shared_ptr<RTBConnectedPlayer> player);
	};

};
