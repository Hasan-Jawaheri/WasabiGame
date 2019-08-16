#pragma once

#include "WasabiGame/Units/UnitsManager.hpp"

enum UNIT_TYPE {
	UNIT_PLAYER = 0,
	UNIT_TEST_BALL = 1,
};

void SetupRTBUnits(UnitsManager* manager, bool isServer);
