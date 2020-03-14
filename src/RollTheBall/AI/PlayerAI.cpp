#include "RollTheBall/AI/PlayerAI.hpp"
#include "RollTheBall/AI/RTBAI.hpp"
#include "RollTheBall/Units/RTBUnits.hpp"


RollTheBall::PlayerAI::PlayerAI(std::shared_ptr<WasabiGame::Unit> unit) : RTBAI(unit) {
	m_camera = unit->GetApp().lock()->CameraManager->GetDefaultCamera();
	m_camera->AddReference();
	m_cameraPivot = WVector3(0.0f, 0.0f, 0.0f);
	m_cameraPitch = 30.0f;
	m_cameraDistance = -10.0f;
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
