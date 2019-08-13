#pragma once

#include "RollTheBall/Units/BallUnit.hpp"
#include "WasabiGame/Units/Units.hpp"
#include "WasabiGame/Units/AI.hpp"

class TestUnitAI : public AI {
public:
	TestUnitAI(class Unit* unit) : AI(unit) {

	}

	virtual ~TestUnitAI() {

	}

	virtual void Update(float fDeltaTime) {

	}
};

