#pragma once

#include "Common.hpp"
#include "WasabiGame/ResourceManager/ResourceManager.hpp"
#include "WasabiGame/UI/UI.hpp"
#include "WasabiGame/Maps/MapLoader.hpp"
#include "WasabiGame/Units/UnitsManager.hpp"
#include "WasabiGame/Utilities/Scheduler.hpp"


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

	public:
		WasabiBaseGame();

		virtual void SwitchToInitialState() = 0;

		virtual WError Setup();
		virtual bool Loop(float fDeltaTime);
		virtual void Cleanup();

		virtual WError Resize(unsigned int width, unsigned int height);
		virtual WError SetupRenderer();
		virtual WPhysicsComponent* CreatePhysicsComponent();

		std::shared_ptr<ResourceManager> Resources;
		std::shared_ptr<UserInterface> UI;
		std::shared_ptr<MapLoader> Maps;
		std::shared_ptr<UnitsManager> Units;
	};

};
