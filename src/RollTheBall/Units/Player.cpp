#include "RollTheBall/Units/Player.hpp"

const char* Player::modelName = "player";

Player::Player() {
	m_camera = APPHANDLE->CameraManager->GetDefaultCamera();
	m_camera->AddReference();
	m_cameraPivot = WVector3(0.0f, 0.0f, 0.0f);

	m_yaw = 0.0f;
	m_pitch = 30.0f;
	m_dist = -10.0f;
	m_draggingCamera = false;
}

Player::~Player() {
	W_SAFE_REMOVEREF(m_camera);
}

void Player::ApplyMousePivot() {
	static bool bMouseHidden = false;
	static int lx, ly;
	if (m_draggingCamera) {
		if (!bMouseHidden) {
			APPHANDLE->WindowAndInputComponent->ShowCursor(false);
			bMouseHidden = true;

			lx = APPHANDLE->WindowAndInputComponent->MouseX(MOUSEPOS_DESKTOP, 0);
			ly = APPHANDLE->WindowAndInputComponent->MouseY(MOUSEPOS_DESKTOP, 0);

			APPHANDLE->WindowAndInputComponent->SetMousePosition(640 / 2, 480 / 2, MOUSEPOS_VIEWPORT);
		}

		int mx = APPHANDLE->WindowAndInputComponent->MouseX(MOUSEPOS_VIEWPORT, 0);
		int my = APPHANDLE->WindowAndInputComponent->MouseY(MOUSEPOS_VIEWPORT, 0);

		int dx = mx - 640 / 2;
		int dy = my - 480 / 2;

		if (fabs(dx) < 2)
			dx = 0;
		if (fabs(dy) < 2)
			dy = 0;

		m_yaw += (float)dx / 2.0f;
		m_pitch += (float)dy / 2.0f;

		if (dx || dy)
			APPHANDLE->WindowAndInputComponent->SetMousePosition(640 / 2, 480 / 2);
	} else {
		if (bMouseHidden) {
			APPHANDLE->WindowAndInputComponent->ShowCursor(true);
			bMouseHidden = false;

			APPHANDLE->WindowAndInputComponent->SetMousePosition(lx, ly, MOUSEPOS_DESKTOP);
		}
	}

	float fMouseZ = (float)APPHANDLE->WindowAndInputComponent->MouseZ();
	m_dist += (fMouseZ / 120.0f) * (abs(m_dist) / 10.0f);
	APPHANDLE->WindowAndInputComponent->SetMouseZ(0);
	m_dist = fmin(-1.0f, m_dist);

	m_camera->SetPosition(m_cameraPivot);
	m_camera->SetAngle(WQuaternion());
	m_camera->Yaw(m_yaw);
	m_camera->Pitch(m_pitch);
	m_camera->Move(m_dist);
}

void Player::BeginDragCamera() {
	m_draggingCamera = true;
}

void Player::EndDragCamera() {
	m_draggingCamera = false;
}

void Player::Update(float fDeltaTime) {
	BallUnit::Update(fDeltaTime);

	// apply mouse rotation pivot
	ApplyMousePivot();

	// make camera follow player's model
	WVector3 rbPos = O()->GetPosition();
	float camToPlayerDistSquare = WVec3LengthSq(rbPos - m_cameraPivot);
	if (camToPlayerDistSquare > 0.05f) {
		m_cameraPivot = m_cameraPivot + (rbPos - m_cameraPivot) * (camToPlayerDistSquare / 2.0f) * fDeltaTime;
	}

	/*static std::array<float, 300> Xs;
	for (uint i = 0; i < Xs.size() - 1; i++)
		Xs[i] = Xs[i + 1];
	Xs[Xs.size() - 1] = rbPos.x;*/
}

PlayerAI::PlayerAI(Unit* unit) : AI(unit) {
	m_isJumpKeyDown = false;
}

PlayerAI::~PlayerAI() {

}

void PlayerAI::Update(float fDeltaTime) {
	Player* player = (Player*)m_unit;
	WVector3 rbPos = player->O()->GetPosition();
	WVector3 direction = WVec3TransformNormal(WVector3(0, 0, 1), WRotationMatrixY(W_DEGTORAD(player->m_yaw)));
	WVector3 right = WVec3TransformNormal(WVector3(1, 0, 0), WRotationMatrixY(W_DEGTORAD(player->m_yaw)));

	WVector3 inputDirection(0, 0, 0);
	if (APPHANDLE->WindowAndInputComponent->KeyDown('W'))
		inputDirection += direction;
	if (APPHANDLE->WindowAndInputComponent->KeyDown('S'))
		inputDirection -= direction;
	if (APPHANDLE->WindowAndInputComponent->KeyDown('A'))
		inputDirection -= right;
	if (APPHANDLE->WindowAndInputComponent->KeyDown('D'))
		inputDirection += right;

	bool isDirection = WVec3LengthSq(inputDirection) > 0.1f;

	if (!APPHANDLE->WindowAndInputComponent->KeyDown(' '))
		m_isJumpKeyDown = true;
	else if (m_isJumpKeyDown) {
		m_isJumpKeyDown = false;
		player->Jump(inputDirection);
	}

	if (isDirection) {
		player->Move(inputDirection);
	}
}
