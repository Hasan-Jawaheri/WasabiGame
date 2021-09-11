#include "RollTheBall/AI/RemoteControlledAI.hpp"


RollTheBall::RemoteControlledAI::RemoteControlledAI(std::shared_ptr<WasabiGame::Unit> unit) : RTBAI(unit) {
	m_lastStateSequenceNumberUsed = 0;
}

RollTheBall::RemoteControlledAI::~RemoteControlledAI() {

}

void RollTheBall::RemoteControlledAI::Update(float fDeltaTime) {
	RollTheBall::UpdateBuilders::GameStateSync::STATE_STRUCT state;
	bool hasNewState = false;
	{
		std::scoped_lock<std::mutex> lock(m_queuedStateUpdatesMutex);
		if (m_queuedStateUpdates.size() > 0) {
			state = m_queuedStateUpdates[0].state;
			m_lastStateSequenceNumberUsed = m_queuedStateUpdates[0].sequenceNumber;
			m_queuedStateUpdates.erase(m_queuedStateUpdates.begin());
			hasNewState = true;
		}
	}

	if (hasNewState) {
		std::shared_ptr<Wasabi> app = m_app.lock();
		std::shared_ptr<WasabiGame::Unit> unit = m_unit.lock();
		float currentTime = app->Timer.GetElapsedTime();
		if (unit->RB()) {
			unit->RB()->SetPosition(state.position);
			unit->RB()->SetAngle(state.rotation);
			unit->RB()->SetLinearVelocity(state.linearVelocity);
			unit->RB()->SetAngularVelocity(state.angularVelocity);
		} else {
			unit->O()->SetPosition(state.position);
			unit->O()->SetAngle(state.rotation);
		}
	}

	RTBAI::Update(fDeltaTime);
}

void RollTheBall::RemoteControlledAI::OnSetMotionState(RollTheBall::UpdateBuilders::GameStateSync::SEQUENCE_NUMBER_TYPE sequenceNumber,
		RollTheBall::UpdateBuilders::GameStateSync::STATE_STRUCT motionState) {
	// TODO: BUFFER THIS (time & queue) ?
	std::scoped_lock<std::mutex> lock(m_queuedStateUpdatesMutex);
	if (sequenceNumber > m_lastStateSequenceNumberUsed) {
		MOTION_STATE_AND_METADATA state;
		state.state = motionState;
		state.sequenceNumber = sequenceNumber;
		m_queuedStateUpdates.push_back(state);
	}
}
