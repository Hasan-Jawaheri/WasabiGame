#pragma once

#include "Common.hpp"
#include "WasabiGame/ResourceManager/ResourceManager.hpp"
#include "WasabiGame/UI/UI.hpp"
#include "WasabiGame/Maps/MapLoader.hpp"
#include "WasabiGame/Units/UnitsManager.hpp"
#include "WasabiGame/Utilities/Scheduler.hpp"
#include "WasabiGame/Utilities/Config.hpp"
#include "WasabiGame/Networking/NetworkManager.hpp"


namespace WasabiGame {

	class WasabiBaseGame : public Wasabi, public std::enable_shared_from_this<WasabiBaseGame> {
	protected:
		struct {
			bool debugVulkan;
			bool debugPhysics;
			int screenWidth;
			int screenHeight;
			bool fullscreen;
			int maxFPS;
			std::string mediaFolder;
		} m_settings;

		std::thread* m_schedulerThread;

	public:
		WasabiBaseGame();

		virtual void SwitchToInitialState() = 0;

		virtual WError Setup() override;
		virtual bool Loop(float fDeltaTime) override;
		virtual void Cleanup() override;

		virtual WError Resize(unsigned int width, unsigned int height) override;
		virtual WError SetupRenderer() override;
		virtual WPhysicsComponent* CreatePhysicsComponent() override;

		std::shared_ptr<GameScheduler> Scheduler;
		std::shared_ptr<GameConfig> Config;
		std::shared_ptr<ResourceManager> Resources;
		std::shared_ptr<UserInterface> UI;
		std::shared_ptr<MapLoader> Maps;
		std::shared_ptr<UnitsManager> Units;
		std::shared_ptr<NetworkManager> Networking;
	};

};
