#pragma once

#include "RollTheBall/AI/RTBAI.hpp"
#include "RollTheBall/Units/BallUnit.hpp"


namespace RollTheBall {

	class TestUnitAI : public RTBAI {
	public:
		TestUnitAI(std::shared_ptr<class WasabiGame::Unit> unit) : RTBAI(unit) {

		}

		virtual ~TestUnitAI() {

		}

		virtual void Update(float fDeltaTime) {

		}
	};

};

