#include "RollTheBall/AI/PlayerAI.hpp"

PlayerAI::PlayerAI(Unit* unit) : RTBAI(unit) {
	m_app = unit->GetApp();
	m_camera = m_app->CameraManager->GetDefaultCamera();
	m_camera->AddReference();
	m_cameraPivot = WVector3(0.0f, 0.0f, 0.0f);

	m_yaw = 0.0f;
	m_pitch = 30.0f;
	m_dist = -10.0f;
	m_draggingCamera = false;
	m_mouseHidden = false;

	m_isJumpKeyDown = false;

	m_updateTimer = m_app->Timer.GetElapsedTime();
}

PlayerAI::~PlayerAI() {
	W_SAFE_REMOVEREF(m_camera);
}

void PlayerAI::Update(float fDeltaTime) {
	Player* player = (Player*)m_unit;
	WVector3 rbPos = player->O()->GetPosition();
	WVector3 direction = WVec3TransformNormal(WVector3(0, 0, 1), WRotationMatrixY(W_DEGTORAD(m_yaw)));
	WVector3 right = WVec3TransformNormal(WVector3(1, 0, 0), WRotationMatrixY(W_DEGTORAD(m_yaw)));

	WVector3 inputDirection(0, 0, 0);
	if (m_unit->GetApp()->WindowAndInputComponent->KeyDown('W'))
		inputDirection += direction;
	if (m_unit->GetApp()->WindowAndInputComponent->KeyDown('S'))
		inputDirection -= direction;
	if (m_unit->GetApp()->WindowAndInputComponent->KeyDown('A'))
		inputDirection -= right;
	if (m_unit->GetApp()->WindowAndInputComponent->KeyDown('D'))
		inputDirection += right;

	bool isDirection = WVec3LengthSq(inputDirection) > 0.1f;

	if (!m_unit->GetApp()->WindowAndInputComponent->KeyDown(' '))
		m_isJumpKeyDown = true;
	else if (m_isJumpKeyDown) {
		m_isJumpKeyDown = false;
		player->Jump(inputDirection);
	}

	if (isDirection) {
		player->Move(inputDirection);
	}

	// apply mouse rotation pivot
	ApplyMousePivot();

	// make camera follow player's model
	WOrientation* orientation = m_unit->O();
	if (orientation) {
		WVector3 rbPos = orientation->GetPosition();
		float camToPlayerDistSquare = WVec3LengthSq(rbPos - m_cameraPivot);
		if (camToPlayerDistSquare > 0.05f) {
			m_cameraPivot = m_cameraPivot + (rbPos - m_cameraPivot) * (camToPlayerDistSquare / 2.0f) * fDeltaTime;
		}

		// send update to server
		if (m_updateTimer + 0.25f < m_app->Timer.GetElapsedTime()) {
			m_updateTimer = m_app->Timer.GetElapsedTime();
			std::function<void(std::string, void*, uint16_t)> addProp = nullptr;
			RTBNet::UpdateBuilders::SetUnitProps(m_unitUpdate, m_unit->GetId(), &addProp);
			addProp("pos", &rbPos, sizeof(WVector3));
			SentNetworkUpdate(m_unitUpdate);
		}
	}
}

void PlayerAI::ApplyMousePivot() {
	if (m_draggingCamera) {
		if (!m_mouseHidden) {
			m_app->WindowAndInputComponent->ShowCursor(false);
			m_mouseHidden = true;

			m_lastMouseX = m_app->WindowAndInputComponent->MouseX(MOUSEPOS_DESKTOP, 0);
			m_lastMouseY = m_app->WindowAndInputComponent->MouseY(MOUSEPOS_DESKTOP, 0);

			m_app->WindowAndInputComponent->SetMousePosition(640 / 2, 480 / 2, MOUSEPOS_VIEWPORT);
		}

		int mx = m_app->WindowAndInputComponent->MouseX(MOUSEPOS_VIEWPORT, 0);
		int my = m_app->WindowAndInputComponent->MouseY(MOUSEPOS_VIEWPORT, 0);

		int dx = mx - 640 / 2;
		int dy = my - 480 / 2;

		if (fabs(dx) < 2)
			dx = 0;
		if (fabs(dy) < 2)
			dy = 0;

		m_yaw += (float)dx / 2.0f;
		m_pitch += (float)dy / 2.0f;

		if (dx || dy)
			m_app->WindowAndInputComponent->SetMousePosition(640 / 2, 480 / 2);
	} else {
		if (m_mouseHidden) {
			m_app->WindowAndInputComponent->ShowCursor(true);
			m_mouseHidden = false;

			m_app->WindowAndInputComponent->SetMousePosition(m_lastMouseX, m_lastMouseY, MOUSEPOS_DESKTOP);
		}
	}

	float fMouseZ = (float)m_app->WindowAndInputComponent->MouseZ();
	m_dist += fMouseZ * (abs(m_dist) / 10.0f);
	m_app->WindowAndInputComponent->SetMouseZ(0);
	m_dist = fmin(-1.0f, m_dist);

	m_camera->SetPosition(m_cameraPivot);
	m_camera->SetAngle(WQuaternion());
	m_camera->Yaw(m_yaw);
	m_camera->Pitch(m_pitch);
	m_camera->Move(m_dist);
}

void PlayerAI::BeginDragCamera() {
	m_draggingCamera = true;
}

void PlayerAI::EndDragCamera() {
	m_draggingCamera = false;
}