#pragma once

#include "RollTheBall/AI/RTBAI.hpp"


namespace RollTheBall {

	class RemoteControlledAI : public RTBAI {
	public:
		RemoteControlledAI(std::shared_ptr<class WasabiGame::Unit> unit);
		virtual ~RemoteControlledAI();

		virtual void Update(float fDeltaTime);
	};

};
