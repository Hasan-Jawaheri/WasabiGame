#pragma once

#include "../../Common.hpp"

class AI {
	class Unit* m_unit;

public:
	AI(class Unit* unit);
	virtual ~AI();

	virtual void Update(float fDeltaTime) = 0;
};