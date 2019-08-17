#pragma once

#include "RollTheBall/AI/RTBAI.hpp"
#include "RollTheBall/Units/BallUnit.hpp"

class TestUnitAI : public RTBAI {
public:
	TestUnitAI(class Unit* unit) : RTBAI(unit) {

	}

	virtual ~TestUnitAI() {

	}

	virtual void Update(float fDeltaTime) {

	}
};

