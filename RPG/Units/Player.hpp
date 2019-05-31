#pragma once

#include "../../WasabiGame/Units/Units.hpp"
#include "../../WasabiGame/Units/AI.hpp"

class Player : public Unit {
public:

	static const char* modelName;

	Player();
	virtual ~Player();
};

class PlayerAI : public AI {
public:
	PlayerAI(class Unit* unit);
	virtual ~PlayerAI();

	virtual void Update(float fDeltaTime);
};

