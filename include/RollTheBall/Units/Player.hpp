#pragma once

#include "RollTheBall/Units/BallUnit.hpp"
#include "WasabiGame/Units/Units.hpp"
#include "WasabiGame/Units/AI.hpp"

class Player : public BallUnit {
	friend class PlayerAI;

protected:

	virtual void Update(float fDeltaTime);

public:

	static const char* modelName;

	Player(Wasabi* app, ResourceManager* resourceManager, class UnitsManager* unitsManager);
	virtual ~Player();
};

