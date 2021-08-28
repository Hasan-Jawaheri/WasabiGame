#include "RollTheBall/AI/PlayerAI.hpp"
#include "RollTheBall/AI/RTBAI.hpp"
#include "RollTheBall/Units/RTBUnits.hpp"
#include "WasabiGame/Main.hpp"
#include "RollTheBall/AI/RemoteControlledAI.hpp"


RollTheBall::PlayerAI::PlayerAI(std::shared_ptr<WasabiGame::Unit> unit) : RTBAI(unit) {
	std::shared_ptr<WasabiGame::WasabiBaseGame> app = unit->GetApp().lock();

	m_camera = app->CameraManager->GetDefaultCamera();
	m_camera->AddReference();
	m_cameraPivot = WVector3(0.0f, 0.0f, 0.0f);
	m_cameraPitch = 30.0f;
	m_cameraDistance = -10.0f;

	m_clientTimer = &unit->GetApp().lock()->Timer;
	m_periodicUpdateTimer = m_clientTimer->GetElapsedTime();
}

RollTheBall::PlayerAI::~PlayerAI() {
	W_SAFE_REMOVEREF(m_camera);
}

void RollTheBall::PlayerAI::Update(float fDeltaTime) {
	std::shared_ptr<Wasabi> app = m_app.lock();
	std::shared_ptr<WasabiGame::Unit> unit = m_unit.lock();

	RTBAI::Update(fDeltaTime);

	m_camera->SetPosition(m_cameraPivot);
	m_camera->SetAngle(WQuaternion());
	m_camera->Yaw(GetYawAngle());
	m_camera->Pitch(m_cameraPitch);
	m_camera->Move(m_cameraDistance);

	// make camera follow player's model
	WOrientation* orientation = unit->O();
	if (orientation) {
		WVector3 rbPos = orientation->GetPosition();
		float camToPlayerDistSquare = WVec3LengthSq(rbPos - m_cameraPivot);
		if (camToPlayerDistSquare > 0.05f) {
			m_cameraPivot = m_cameraPivot + (rbPos - m_cameraPivot) * (camToPlayerDistSquare / 2.0f) * fDeltaTime;
		}
	}

	// send periodic update to server
	/*if (m_periodicUpdateTimer + 0.2f < m_clientTimer->GetElapsedTime()) {
		m_periodicUpdateTimer = m_clientTimer->GetElapsedTime();

		SendMovementUpdate('P', m_playerPosition);
	}*/
}

void RollTheBall::PlayerAI::SetCameraPitch(float pitch) {
	m_cameraPitch = pitch;
}

void RollTheBall::PlayerAI::SetCameraDistance(float distance) {
	m_cameraDistance= distance;
}

float RollTheBall::PlayerAI::GetCameraPitch() const {
	return m_cameraPitch;
}

float RollTheBall::PlayerAI::GetCameraDistance() const {
	return m_cameraDistance;
}

void RollTheBall::PlayerAI::OnNetworkUpdate(std::string prop, void* data, size_t size) {
}

void RollTheBall::PlayerAI::SendMovementUpdate(void* update, size_t size) {
	std::function<void(std::string, void*, uint16_t)> setProp;
	RollTheBall::UpdateBuilders::SetUnitProps(m_update, 0, &setProp);
	setProp("move", update, size);
	SendNetworkUpdate(m_update);
}

void RollTheBall::PlayerAI::SendMovementUpdate(char type, float angle) {
	RollTheBall::MOVEMENT_PACKET_STRUCT m;
	m.type = type;
	m.time = m_clientTimer->GetElapsedTime();
	m.prop.angle = angle;
	SendMovementUpdate((void*)&m, sizeof(RollTheBall::MOVEMENT_PACKET_STRUCT));
}

void RollTheBall::PlayerAI::SendMovementUpdate(char type, WVector3 position) {
	RollTheBall::MOVEMENT_PACKET_STRUCT m;
	m.type = type;
	m.time = m_clientTimer->GetElapsedTime();
	m.prop.pos = position;
	SendMovementUpdate((void*)&m, sizeof(RollTheBall::MOVEMENT_PACKET_STRUCT));
}

void RollTheBall::PlayerAI::SendMovementUpdate(char type, bool state) {
	RollTheBall::MOVEMENT_PACKET_STRUCT m;
	m.type = type;
	m.time = m_clientTimer->GetElapsedTime();
	m.prop.state = state;
	SendMovementUpdate((void*)&m, sizeof(RollTheBall::MOVEMENT_PACKET_STRUCT));
}

void RollTheBall::PlayerAI::SetYawAngle(float angle) {
	RTBAI::SetYawAngle(angle);

	SendMovementUpdate('Y', angle);
}

void RollTheBall::PlayerAI::SetMoveForward(bool isActive) {
	RTBAI::SetMoveForward(isActive);

	SendMovementUpdate('W', isActive);
}

void RollTheBall::PlayerAI::SetMoveBackward(bool isActive) {
	RTBAI::SetMoveBackward(isActive);

	SendMovementUpdate('S', isActive);
}

void RollTheBall::PlayerAI::SetMoveLeft(bool isActive) {
	RTBAI::SetMoveLeft(isActive);

	SendMovementUpdate('A', isActive);
}

void RollTheBall::PlayerAI::SetMoveRight(bool isActive) {
	RTBAI::SetMoveRight(isActive);

	SendMovementUpdate('D', isActive);
}

void RollTheBall::PlayerAI::SetMoveJump(bool isActive) {
	RTBAI::SetMoveJump(isActive);

	SendMovementUpdate(' ', isActive);
}

