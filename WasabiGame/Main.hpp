#pragma once

#include "..//WasabiGame/Common.hpp"

class WasabiRPG : public Wasabi {
protected:
	struct {
		bool debugVulkan;
		bool debugPhysics;
		int screenWidth;
		int screenHeight;
		bool fullscreen;
		int maxFPS;
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
};
