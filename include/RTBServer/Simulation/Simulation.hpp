#pragma once

#include "RollTheBall/Utilities/Scheduler.hpp"
#include "RTBServer/Main.hpp"
#include "RTBServer/Game/RTBConnectedPlayer.hpp"

#include <atomic>

class ServerSimulation : public HBUtils::SchedulerThread {
	friend class SimulationWasabi;
	friend class SimulationGameState;

	void* m_simulationWasabi;
	std::atomic<void*> m_gameState;
	bool m_simulationLoaded;
	class RTBServer* m_server;
	bool m_generateAssets;

	void WaitForSimulationLaunch();

public:
	ServerSimulation(class RTBServer* server, bool generateAssets = true);

	void Run();

	void AddPlayer(std::shared_ptr<RTBConnectedPlayer> player);
	void RemovePlayer(std::shared_ptr<RTBConnectedPlayer> player);
};
