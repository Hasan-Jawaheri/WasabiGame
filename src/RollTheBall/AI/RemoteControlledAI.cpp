#include "RollTheBall/AI/RemoteControlledAI.hpp"


RollTheBall::RemoteControlledAI::RemoteControlledAI(std::shared_ptr<WasabiGame::Unit> unit) : RTBAI(unit) {
}

RollTheBall::RemoteControlledAI::~RemoteControlledAI() {

}

void RollTheBall::RemoteControlledAI::Update(float fDeltaTime) {
	std::shared_ptr<Wasabi> app = m_app.lock();
	std::optional<RollTheBall::UpdateBuilders::GameStateSync::STATE_STRUCT> stateOptional;
	{
		std::scoped_lock<std::mutex> lock(m_queuedStateUpdatesMutex);
		stateOptional = m_queuedStateUpdates.ConsumePacket(app->Timer.GetElapsedTime());
	}

	if (stateOptional.has_value()) {
		RollTheBall::UpdateBuilders::GameStateSync::STATE_STRUCT state = stateOptional.value();
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
	std::scoped_lock<std::mutex> lock(m_queuedStateUpdatesMutex);
	m_queuedStateUpdates.InsertPacket(m_app.lock()->Timer.GetElapsedTime(), motionState, sequenceNumber, std::numeric_limits<uint16_t>::max());
}
