#include "RollTheBall/AI/PlayerAI.hpp"
#include "RollTheBall/AI/RTBAI.hpp"
#include "RollTheBall/Units/RTBUnits.hpp"
#include "WasabiGame/Main.hpp"
#include "RollTheBall/AI/RemoteControlledAI.hpp"
#include "Wasabi/Core/WTimer.hpp"


RollTheBall::PlayerAI::PlayerAI(std::shared_ptr<WasabiGame::Unit> unit)
		: RemoteControlledAI(unit), m_inputSequenceNumber(0), m_update({ 0 }) {
	std::shared_ptr<WasabiGame::WasabiBaseGame> app = unit->GetApp().lock();

	m_camera = app->CameraManager->GetDefaultCamera();
	m_camera->AddReference();
	m_cameraPivot = WVector3(0.0f, 0.0f, 0.0f);
	m_cameraPitch = 30.0f;
	m_cameraDistance = -10.0f;

	m_clientTimer = &unit->GetApp().lock()->Timer;
	m_sendInputTimer = m_clientTimer->GetElapsedTime();
	m_packetsToSend.reserve(RollTheBall::UpdateBuilders::GameStateSync::MAX_INPUTS_PER_PACKET);
	m_lastInputRecordedTime = -100000000.0f;
}

RollTheBall::PlayerAI::~PlayerAI() {
	W_SAFE_REMOVEREF(m_camera);
}

void RollTheBall::PlayerAI::Update(float fDeltaTime) {
	std::shared_ptr<Wasabi> app = m_app.lock();
	std::shared_ptr<WasabiGame::Unit> unit = m_unit.lock();

	RemoteControlledAI::Update(fDeltaTime);

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
			m_cameraPivot = m_cameraPivot + (rbPos - m_cameraPivot) * std::min((camToPlayerDistSquare / 2.0f) * fDeltaTime, 0.8f);
		}
	}

	float curTime = m_clientTimer->GetElapsedTime(true);
	if (m_sendInputTimer + SEND_INPUT_TO_SERVER_PERIOD_S < curTime) {
		std::scoped_lock<std::mutex> lockGuard(m_inputsMutex);
		m_sendInputTimer = curTime;
		if (m_packetsToSend.size() > 0) {
			RollTheBall::UpdateBuilders::GameStateSync::SetPlayerInput(m_update, m_packetsToSend);
			SendNetworkUpdate(m_update, false);
			for (uint32_t i = 0; i < m_packetsToSendMetadata.size(); i++)
				m_packetsToSendMetadata[i].sent = true;
		}
	}
}

void RollTheBall::PlayerAI::SetCameraPitch(float pitch) {
	m_cameraPitch = pitch;
}

void RollTheBall::PlayerAI::SetCameraDistance(float distance) {
	m_cameraDistance = distance;
}

float RollTheBall::PlayerAI::GetCameraPitch() const {
	return m_cameraPitch;
}

float RollTheBall::PlayerAI::GetCameraDistance() const {
	return m_cameraDistance;
}

void RollTheBall::PlayerAI::QueueMovementUpdate() {
	IN_FLIGHT_INPUT_STRUCT inputMetadata;
	inputMetadata.sent = false;
	RollTheBall::UpdateBuilders::GameStateSync::INPUT_STRUCT input = { 0 };
	input.forward = m_movement.forward;
	input.backward = m_movement.backward;
	input.left = m_movement.left;
	input.right = m_movement.right;
	input.jump = m_movement.jump;
	input.yaw = m_movement.angle;

	{
		std::scoped_lock<std::mutex> lockGuard(m_inputsMutex);
		if (m_packetsToSendMetadata.size() > 0 && !m_packetsToSendMetadata[m_packetsToSendMetadata.size() - 1].sent) {
			// if last packet is not sent and is same as this one except for angle, just merge the two
			RollTheBall::UpdateBuilders::GameStateSync::INPUT_STRUCT* lastPacket = &m_packetsToSend[m_packetsToSend.size() - 1];
			if (lastPacket->backward == input.backward && lastPacket->forward == input.forward &&
					lastPacket->left == input.left && lastPacket->right == input.right && lastPacket->jump == input.jump) {
				lastPacket->yaw = input.yaw;
				return;
			}
		}

		uint16_t millisSinceLastInput = (uint16_t)(m_clientTimer->GetElapsedTime() - m_lastInputRecordedTime) * 1000.0f;
		m_lastInputRecordedTime = m_clientTimer->GetElapsedTime();
		if (millisSinceLastInput >= std::numeric_limits<uint16_t>::max() - 1)
			input.millisSinceLastInput = std::numeric_limits<uint16_t>::max();
		else
			input.millisSinceLastInput = millisSinceLastInput;
		input.sequenceNumber = m_inputSequenceNumber++;
		m_packetsToSendMetadata.push_back(inputMetadata);
		m_packetsToSend.push_back(input);
		if (m_packetsToSend.size() > RollTheBall::UpdateBuilders::GameStateSync::MAX_INPUTS_PER_PACKET) {
			m_packetsToSend.erase(m_packetsToSend.begin());
			m_packetsToSendMetadata.erase(m_packetsToSendMetadata.begin());
		}
	}
}

void RollTheBall::PlayerAI::SetYawAngle(float angle) {
	RemoteControlledAI::SetYawAngle(angle);
	QueueMovementUpdate();
}

void RollTheBall::PlayerAI::SetMoveForward(bool isActive) {
	RemoteControlledAI::SetMoveForward(isActive);
	QueueMovementUpdate();
}

void RollTheBall::PlayerAI::SetMoveBackward(bool isActive) {
	RemoteControlledAI::SetMoveBackward(isActive);
	QueueMovementUpdate();
}

void RollTheBall::PlayerAI::SetMoveLeft(bool isActive) {
	RemoteControlledAI::SetMoveLeft(isActive);
	QueueMovementUpdate();
}

void RollTheBall::PlayerAI::SetMoveRight(bool isActive) {
	RemoteControlledAI::SetMoveRight(isActive);
	QueueMovementUpdate();
}

void RollTheBall::PlayerAI::SetMoveJump(bool isActive) {
	RemoteControlledAI::SetMoveJump(isActive);
	QueueMovementUpdate();
}

void RollTheBall::PlayerAI::OnInputsAcked(std::vector<SEQUENCE_NUMBER_TYPE> inputs) {
	std::scoped_lock<std::mutex> lockGuard(m_inputsMutex);
	for (int i = 0; i < m_packetsToSend.size(); i++) {
		bool iIsAcked = false;
		for (int j = 0; j < inputs.size() && !iIsAcked; j++) {
			if (m_packetsToSend[i].sequenceNumber == inputs[j]) {
				iIsAcked = true;
			}
		}
		if (iIsAcked) {
			m_packetsToSend.erase(m_packetsToSend.begin() + i);
			m_packetsToSendMetadata.erase(m_packetsToSendMetadata.begin() + i);
			i--;
		}
	}
}
