#pragma once

#include "RollTheBall/Units/BallUnit.hpp"
#include "WasabiGame/Units/Units.hpp"
#include "WasabiGame/Units/AI.hpp"


namespace RollTheBall {

	class Player : public BallUnit {
		friend class PlayerAI;

	protected:

		virtual void Update(float fDeltaTime);

	public:

		static const char* modelName;

		Player(std::shared_ptr<WasabiGame::WasabiBaseGame> app, std::shared_ptr<class WasabiGame::ResourceManager> resourceManager, std::shared_ptr<class WasabiGame::UnitsManager> unitsManager);
		virtual ~Player();
	};

};
