#pragma once

#include "RollTheBall/AI/RTBAI.hpp"


namespace RollTheBall {

	class RemoteControlledAI : public RTBAI {

		std::mutex m_queuedStateUpdatesMutex;
		RollTheBall::UpdateBuilders::GameStateSync::PacketsBuffer<RollTheBall::UpdateBuilders::GameStateSync::STATE_STRUCT> m_queuedStateUpdates;

	public:
		RemoteControlledAI(std::shared_ptr<class WasabiGame::Unit> unit);
		virtual ~RemoteControlledAI();

		virtual void Update(float fDeltaTime) override;

		virtual void OnSetMotionState(RollTheBall::UpdateBuilders::GameStateSync::SEQUENCE_NUMBER_TYPE sequenceNumber,
			RollTheBall::UpdateBuilders::GameStateSync::STATE_STRUCT motionState) override;
	};

};
