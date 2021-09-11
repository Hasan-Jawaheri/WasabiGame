#pragma once

#include "RollTheBall/AI/RTBAI.hpp"


namespace RollTheBall {

	class RemoteControlledAI : public RTBAI {

		struct MOTION_STATE_AND_METADATA {
			RollTheBall::UpdateBuilders::GameStateSync::STATE_STRUCT state;
			RollTheBall::UpdateBuilders::GameStateSync::SEQUENCE_NUMBER_TYPE sequenceNumber;
		};

		std::mutex m_queuedStateUpdatesMutex;
		std::vector<MOTION_STATE_AND_METADATA> m_queuedStateUpdates;
		RollTheBall::UpdateBuilders::GameStateSync::SEQUENCE_NUMBER_TYPE m_lastStateSequenceNumberUsed;

	public:
		RemoteControlledAI(std::shared_ptr<class WasabiGame::Unit> unit);
		virtual ~RemoteControlledAI();

		virtual void Update(float fDeltaTime) override;

		virtual void OnSetMotionState(RollTheBall::UpdateBuilders::GameStateSync::SEQUENCE_NUMBER_TYPE sequenceNumber,
			RollTheBall::UpdateBuilders::GameStateSync::STATE_STRUCT motionState) override;
	};

};
