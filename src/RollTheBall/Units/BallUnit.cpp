#include "RollTheBall/Units/BallUnit.hpp"
#include "WasabiGame/Units/UnitsManager.hpp"


const char* RollTheBall::BallUnit::modelName = "small-ball";

RollTheBall::BallUnit::BallUnit(std::shared_ptr<WasabiGame::WasabiBaseGame> app, std::shared_ptr<class WasabiGame::ResourceManager> resourceManager, std::shared_ptr<class WasabiGame::UnitsManager> unitsManager) : WasabiGame::Unit(app, resourceManager, unitsManager) {
	m_state.isGrounded = false;
	m_state.didDash = true;
	m_state.jumpDirection = WVector3(0.0f, 0.0f, 0.0f);

	m_properties.canJump = true;
	m_properties.canDash = true;
	m_properties.movementSpeed = 500.0f;
	m_properties.jumpHeight = 300.0f;
	m_properties.dashSpeed = 300.0f;
}

RollTheBall::BallUnit::~BallUnit() {

}

bool RollTheBall::BallUnit::IsOnGround() const {
	return m_state.isGrounded;
}

void RollTheBall::BallUnit::Update(float fDeltaTime) {
	WasabiGame::WasabiBaseGame* app = m_app.lock().get();

	WRigidBody* rb = RB();
	if (rb) {
		WVector3 rbPos = O()->GetPosition();
		m_state.isGrounded = app->PhysicsComponent->RayCast(rbPos + WVector3(0.0f, -0.98f, 0.0f), rbPos + WVector3(0.0f, -1.2f, 0.0f));

		if (m_state.isGrounded)
			rb->SetLinearDamping(0.8f);
		else
			rb->SetLinearDamping(0.2f);
	}

	Unit::Update(fDeltaTime);
}

void RollTheBall::BallUnit::Jump(WVector3 direction) {
	WRigidBody* rb = RB();
	if (rb) {
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
}

void RollTheBall::BallUnit::Move(WVector3 direction) {
	if (m_state.isGrounded) {
		WRigidBody* rb = RB();
		if (rb) {
			rb->ApplyForce(WVec3Normalize(direction) * m_properties.movementSpeed, WVector3(0.0f, 0.3f, 0.0f));
		}
	}
}
