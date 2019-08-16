#pragma once

#include "RollTheBall/Utilities/Scheduler.hpp"
#include "RTBServer/Game/Game.hpp"
#include "RTBServer/Game/Player.hpp"

#include <atomic>

class ServerSimulation : public HBUtils::SchedulerThread {
	friend class SimulationWasabi;
	friend class SimulationGameState;

	void* m_simulationWasabi;
	std::atomic<void*> m_gameState;
	bool m_simulationLoaded;
	class RTBGame* m_game;
	bool m_generateAssets;

	void WaitForSimulationLaunch();

public:
	ServerSimulation(class RTBGame* game, bool generateAssets = true);

	void Run();

	void AddPlayer(std::shared_ptr<RTBPlayer> player);
	void RemovePlayer(std::shared_ptr<RTBPlayer> player);
};
