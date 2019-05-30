#pragma once

#include "..//WasabiGame/Common.hpp"

class WasabiRPG : public Wasabi {

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
