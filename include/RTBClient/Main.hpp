#pragma once

#include "RollTheBall/Main.hpp"
#include "WasabiGame/Utilities/Scheduler.hpp"
#include "WasabiGame/Utilities/Config.hpp"

#include "RTBClient/Networking/Networking.hpp"


namespace RTBClient {

	class ClientNetworking;

	class ClientApplication : public WasabiGame::WasabiBaseGame {
	public:
		ClientApplication(bool generateAssets = true, bool enableVulkanDebugging = true, bool enablePhysicsDebugging = true);
		~ClientApplication();

		std::shared_ptr<WasabiGame::GameConfig> Config;
		std::shared_ptr<WasabiGame::GameScheduler> Scheduler;
		std::shared_ptr<ClientNetworking> Networking;

		void SwitchToInitialState();
	};

};
