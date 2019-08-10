#pragma once

#include "../Lib/Server.hpp"
#include "../Lib/Utilities/Scheduler.hpp"

namespace RPGNet {
	class ServerSimulation : public HBUtils::SchedulerThread {
		Server* m_server;

	public:
		ServerSimulation(Server* server);

		void Run();
	};
};
