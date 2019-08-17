#pragma once

#include "RollTheBall/AI/RTBAI.hpp"

class RemoteControlledAI : public RTBAI {
public:
	RemoteControlledAI(class Unit* unit);
	virtual ~RemoteControlledAI();

	virtual void Update(float fDeltaTime);
};

