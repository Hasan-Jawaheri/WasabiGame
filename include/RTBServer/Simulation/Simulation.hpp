#pragma once

#include "RollTheBall/Networking/Server.hpp"
#include "RollTheBall/Utilities/Scheduler.hpp"

namespace RPGNet {
	class ServerSimulation : public HBUtils::SchedulerThread {
		Server* m_server;

	public:
		ServerSimulation(Server* server);

		void Run();
	};
};
