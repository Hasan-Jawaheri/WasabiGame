#pragma once

#include "RollTheBall/Networking/Server.hpp"

#include "RTBServer/Game/Player.hpp"

class ServerSimulation : public HBUtils::SchedulerThread {
	friend class SimulationWasabi;
	void* m_simulationWasabi;
	void* m_gameState;
	RPGNet::Server* m_server;
	bool m_generateAssets;

	void WaitForSimulationLaunch();

public:
	ServerSimulation(RPGNet::Server* server, bool generateAssets = true);

	void Run();

	void AddPlayer(RTBPlayer* player);
};
