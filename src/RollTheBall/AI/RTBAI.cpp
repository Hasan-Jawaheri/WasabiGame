#include "RollTheBall/AI/RTBAI.hpp"
#include "RollTheBall/Main.hpp"


RollTheBall::RTBAI::RTBAI(std::shared_ptr<WasabiGame::Unit> unit) : AI(unit) {
	m_app = unit->GetApp();
	m_updateTimer = m_app.lock().get()->Timer.GetElapsedTime();

	memset(&m_movement, 0, sizeof(m_movement));
}

RollTheBall::RTBAI::~RTBAI() {

}

void RollTheBall::RTBAI::Update(float fDeltaTime) {
	std::shared_ptr<Wasabi> app = m_app.lock();
	std::shared_ptr<WasabiGame::Unit> unit = m_unit.lock();

	WVector3 rbPos = unit->O()->GetPosition();
	WVector3 direction = WVec3TransformNormal(WVector3(0, 0, 1), WRotationMatrixY(W_DEGTORAD(m_movement.angle)));
	WVector3 right = WVec3TransformNormal(WVector3(1, 0, 0), WRotationMatrixY(W_DEGTORAD(m_movement.angle)));

	WVector3 inputDirection(0, 0, 0);
	if (m_movement.forward)
		inputDirection += direction;
	if (m_movement.backward)
		inputDirection -= direction;
	if (m_movement.left)
		inputDirection -= right;
	if (m_movement.right)
		inputDirection += right;

	bool isDirection = WVec3LengthSq(inputDirection) > 0.1f;

	if (!m_movement.jump)
		m_movement.jumpReady = true;
	else if (m_movement.jumpReady) {
		m_movement.jumpReady = false;
		unit->Jump(inputDirection);
	}

	if (isDirection) {
		unit->Move(inputDirection);
	}

	// send update to server
	if (m_updateTimer + 0.01f < app->Timer.GetElapsedTime()) {
		m_updateTimer = app->Timer.GetElapsedTime();
		WVector3 rbPos = unit->O()->GetPosition();
		std::function<void(std::string, void*, uint16_t)> addProp = nullptr;
		RollTheBall::UpdateBuilders::SetUnitProps(m_unitUpdate, unit->GetId(), &addProp);
		addProp("pos", &rbPos, sizeof(WVector3));
		SendNetworkUpdate(m_unitUpdate);
	}
}

void RollTheBall::RTBAI::SendNetworkUpdate(WasabiGame::NetworkUpdate& update) {
	WasabiGame::WasabiBaseGame* app = m_app.lock().get();
	app->Networking->SendUpdate(update);
}

void RollTheBall::RTBAI::OnNetworkUpdate(std::string prop, void* data, size_t size) {
	std::shared_ptr<WasabiGame::Unit> unit = m_unit.lock();
	WOrientation* orientation = unit->O();

	if (prop == "pos" && size == sizeof(WVector3)) {
		WVector3 pos;
		memcpy(&pos, data, sizeof(WVector3));
		if (orientation)
			orientation->SetPosition(pos);
	}
}

void RollTheBall::RTBAI::SetYawAngle(float angle) {
	m_movement.angle = angle;
}

void RollTheBall::RTBAI::SetMoveForward(bool isActive) {
	m_movement.forward = isActive;
}

void RollTheBall::RTBAI::SetMoveBackward(bool isActive) {
	m_movement.backward = isActive;
}

void RollTheBall::RTBAI::SetMoveLeft(bool isActive) {
	m_movement.left = isActive;
}

void RollTheBall::RTBAI::SetMoveRight(bool isActive) {
	m_movement.right = isActive;
}

void RollTheBall::RTBAI::SetMoveJump(bool isActive) {
	m_movement.jump = isActive;
}

float RollTheBall::RTBAI::GetYawAngle() const {
	return m_movement.angle;
}

bool RollTheBall::RTBAI::GetMoveForward() const {
	return m_movement.forward;
}

bool RollTheBall::RTBAI::GetMoveBackward() const {
	return m_movement.backward;
}

bool RollTheBall::RTBAI::GetMoveLeft() const {
	return m_movement.left;
}

bool RollTheBall::RTBAI::GetMoveRight() const {
	return m_movement.right;
}

bool RollTheBall::RTBAI::GetMoveJump() const {
	return m_movement.jump;
}

