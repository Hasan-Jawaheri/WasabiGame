#include "BallUnit.hpp"

BallUnit::BallUnit() : Unit() {
	m_state.isGrounded = false;
	m_state.didDash = true;
	m_state.jumpDirection = WVector3(0.0f, 0.0f, 0.0f);

	m_properties.canJump = true;
	m_properties.canDash = true;
	m_properties.movementSpeed = 500.0f;
	m_properties.jumpHeight = 300.0f;
	m_properties.dashSpeed = 300.0f;
}

BallUnit::~BallUnit() {

}

bool BallUnit::IsOnGround() const {
	return m_state.isGrounded;
}

void BallUnit::Update(float fDeltaTime) {
	WRigidBody* rb = RB();
	WVector3 rbPos = O()->GetPosition();
	m_state.isGrounded = APPHANDLE->PhysicsComponent->RayCast(rbPos + WVector3(0.0f, -0.98f, 0.0f), rbPos + WVector3(0.0f, -1.2f, 0.0f));

	if (m_state.isGrounded)
		rb->SetLinearDamping(0.8f);
	else
		rb->SetLinearDamping(0.2f);

	Unit::Update(fDeltaTime);
}

void BallUnit::Jump(WVector3 direction) {
	WRigidBody* rb = RB();
	bool isDirection = WVec3LengthSq(direction) > 0.1f;

	if (m_state.isGrounded && m_properties.canJump) {
		rb->SetLinearDamping(0.2f);
		rb->ApplyImpulse(WVector3(0.0f, m_properties.jumpHeight, 0.0f));
		m_state.jumpDirection = direction;
		m_state.didDash = false;
	} else if (!m_state.didDash && isDirection && m_properties.canDash) {
		rb->ApplyImpulse(direction * m_properties.dashSpeed);
		m_state.didDash = true;
	}
}

void BallUnit::Move(WVector3 direction) {
	if (m_state.isGrounded) {
		WRigidBody* rb = RB();
		rb->ApplyForce(WVec3Normalize(direction) * m_properties.movementSpeed, WVector3(0.0f, 0.3f, 0.0f));
	}
}
