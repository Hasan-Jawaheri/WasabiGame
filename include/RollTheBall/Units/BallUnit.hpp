#pragma once

#include "WasabiGame/Units/Units.hpp"

class BallUnit : public Unit {
	struct {
		bool isGrounded;
		bool didDash;
		WVector3 jumpDirection;
	} m_state;

protected:

	virtual void Update(float fDeltaTime);

public:
	BallUnit();
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

