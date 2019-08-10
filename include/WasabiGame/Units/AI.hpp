#pragma once

#include "Common.hpp"

class AI {
	friend class Unit;

protected:
	class Unit* m_unit;

	AI(class Unit* unit);
	virtual ~AI();

public:
	virtual void Update(float fDeltaTime) = 0;
};