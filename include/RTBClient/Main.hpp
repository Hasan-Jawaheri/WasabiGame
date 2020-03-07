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

		void SwitchToInitialState();
	};

};
