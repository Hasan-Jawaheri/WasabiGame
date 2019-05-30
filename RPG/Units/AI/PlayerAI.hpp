#pragma once

#include "../../../WasabiGame/Units/AI.hpp"

class PlayerAI : public AI {
public:
	PlayerAI(class Unit* unit);
	virtual ~PlayerAI();

	virtual void Update(float fDeltaTime);
};
