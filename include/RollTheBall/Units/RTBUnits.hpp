#pragma once

#include "WasabiGame/Units/UnitsManager.hpp"


namespace RollTheBall {

	enum UNIT_TYPE {
		UNIT_PLAYER = 0, // The player's unit
		UNIT_OTHER_PLAYER = 1, // another player's unit
		UNIT_TEST_BALL = 2,
	};

	void SetupRTBUnits(std::shared_ptr<WasabiGame::UnitsManager> manager, bool isServer);

};
