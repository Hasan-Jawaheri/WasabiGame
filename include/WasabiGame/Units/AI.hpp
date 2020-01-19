#pragma once

#include "Common.hpp"


namespace WasabiGame {

	class Unit;

	class AI {
		friend class Unit;

	protected:
		std::shared_ptr<Unit> m_unit;

		AI(std::shared_ptr<Unit> unit);
		virtual ~AI();

	public:
		virtual void Update(float fDeltaTime) = 0;
	};

};
