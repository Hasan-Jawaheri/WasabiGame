#pragma once

#include "../../WasabiGame/Units/Units.hpp"

class Player : public Unit {
public:

	static const char* modelName;

	Player();
	virtual ~Player();
};

