#pragma once

#include "RollTheBall/Utilities/Scheduler.hpp"
#include "RTBServer/Game/Game.hpp"
#include "RTBServer/Game/Player.hpp"

class ServerSimulation : public HBUtils::SchedulerThread {
	friend class SimulationWasabi;
	void* m_simulationWasabi;
	void* m_gameState;
	class RTBGame* m_game;
	bool m_generateAssets;

	void WaitForSimulationLaunch();

public:
	ServerSimulation(class RTBGame* game, bool generateAssets = true);

	void Run();

	void AddPlayer(std::shared_ptr<RTBPlayer> player);
	void RemovePlayer(std::shared_ptr<RTBPlayer> player);
};
