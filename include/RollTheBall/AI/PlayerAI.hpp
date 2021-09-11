#pragma once

#include "RollTheBall/AI/RTBAI.hpp"
#include "RollTheBall/AI/RemoteControlledAI.hpp"
#include "RollTheBall/Units/Player.hpp"
#include "RollTheBall/Networking/GameStateSyncProtocol.hpp"


class WTimer;
using RollTheBall::UpdateBuilders::GameStateSync::SEQUENCE_NUMBER_TYPE;

namespace RollTheBall {

	const float SEND_INPUT_TO_SERVER_PERIOD_S = 0.1f;

	struct IN_FLIGHT_INPUT_STRUCT {
		bool sent;
	};
	
	class PlayerAI : public RemoteControlledAI {
		WCamera* m_camera;
		WVector3 m_cameraPivot;
		float m_cameraPitch, m_cameraDistance;

		WTimer* m_clientTimer;
		float m_sendInputTimer;
		SEQUENCE_NUMBER_TYPE m_inputSequenceNumber;
		std::vector<RollTheBall::UpdateBuilders::GameStateSync::INPUT_STRUCT> m_packetsToSend;
		std::vector<IN_FLIGHT_INPUT_STRUCT> m_packetsToSendMetadata;
		std::mutex m_inputsMutex;
		WasabiGame::NetworkUpdate m_update;

		void QueueMovementUpdate();

	public:
		PlayerAI(std::shared_ptr<class WasabiGame::Unit> unit);
		virtual ~PlayerAI();

		virtual void Update(float fDeltaTime);

		void SetCameraPitch(float pitch);
		void SetCameraDistance(float distance);
		float GetCameraPitch() const;
		float GetCameraDistance() const;

		virtual void SetYawAngle(float angle) override;
		virtual void SetMoveForward(bool isActive) override;
		virtual void SetMoveBackward(bool isActive) override;
		virtual void SetMoveLeft(bool isActive) override;
		virtual void SetMoveRight(bool isActive) override;
		virtual void SetMoveJump(bool isActive) override;

		void OnInputsAcked(std::vector<SEQUENCE_NUMBER_TYPE> inputs);
	};

};
