#pragma once

#include "WasabiGame/Main.hpp"
#include "WasabiGame/Units/Units.hpp"


namespace RollTheBall {

	class BallUnit : public WasabiGame::Unit {
		struct {
			bool isGrounded;
			bool didDash;
			WVector3 jumpDirection;
		} m_state;

	protected:

		virtual void Update(float fDeltaTime);

	public:

		static const char* modelName;

		BallUnit(std::shared_ptr<WasabiGame::WasabiBaseGame> app, std::shared_ptr<class WasabiGame::ResourceManager> resourceManager, std::shared_ptr<class WasabiGame::UnitsManager> unitsManager);
		virtual ~BallUnit();

		struct {
			bool canJump;
			bool canDash;
			float movementSpeed;
			float jumpHeight;
			float dashSpeed;
		} m_properties;

		bool IsOnGround() const;
		void Jump(WVector3 direction);
		void Move(WVector3 direction);
	};

};
