#pragma once

#include "Common.hpp"
#include "WasabiGame/ResourceManager/ResourceManager.hpp"
#include "WasabiGame/UI/UI.hpp"
#include "WasabiGame/Maps/MapLoader.hpp"
#include "WasabiGame/Units/UnitsManager.hpp"

class WasabiRPG : public Wasabi {
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
	WasabiRPG();

	virtual void SwitchToInitialState() = 0;

	virtual WError Setup();
	virtual bool Loop(float fDeltaTime);
	virtual void Cleanup();

	virtual WError Resize(unsigned int width, unsigned int height);
	virtual WError SetupRenderer();
	virtual WPhysicsComponent* CreatePhysicsComponent();

	float fCurGameTime;

	ResourceManager* Resources;
	UserInterface* UI;
	MapLoader* Maps;
	UnitsManager* Units;
};
