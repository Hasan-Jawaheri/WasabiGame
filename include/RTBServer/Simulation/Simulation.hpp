#pragma once

#include "RollTheBall/Networking/Server.hpp"
#include "RollTheBall/Utilities/Scheduler.hpp"

class ServerSimulation : public HBUtils::SchedulerThread {
	RPGNet::Server* m_server;
	bool m_generateAssets;

public:
	ServerSimulation(RPGNet::Server* server, bool generateAssets = true);

	void Run();
};
