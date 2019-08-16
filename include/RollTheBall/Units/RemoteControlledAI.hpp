#pragma once

#include "WasabiGame/Units/Units.hpp"
#include "WasabiGame/Units/AI.hpp"

class RemoteControlledAI : public AI {
public:
	RemoteControlledAI(class Unit* unit);
	virtual ~RemoteControlledAI();

	virtual void Update(float fDeltaTime);
};

