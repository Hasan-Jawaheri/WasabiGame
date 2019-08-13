#pragma once

#include "RollTheBall/Networking/Server.hpp"
#include "RollTheBall/Utilities/Scheduler.hpp"

namespace RPGNet {
	class ServerSimulation : public HBUtils::SchedulerThread {
		Server* m_server;
		bool m_generateAssets;

	public:
		ServerSimulation(Server* server, bool generateAssets = true);

		void Run();
	};
};
