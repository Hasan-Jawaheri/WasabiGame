#include "RollTheBall/AI/RemoteControlledAI.hpp"


RollTheBall::RemoteControlledAI::RemoteControlledAI(std::shared_ptr<WasabiGame::Unit> unit) : RTBAI(unit) {

}

RollTheBall::RemoteControlledAI::~RemoteControlledAI() {

}

void RollTheBall::RemoteControlledAI::Update(float fDeltaTime) {
	std::shared_ptr<Wasabi> app = m_app.lock();
	std::shared_ptr<WasabiGame::Unit> unit = m_unit.lock();

	float currentTime = app->Timer.GetElapsedTime();
	while (m_replayStates.size() > 0 && m_replayStates[0].time + 0.2f <= currentTime) {
		WOrientation* orientation = unit->O();
		RollTheBall::MOVEMENT_PACKET_STRUCT m = m_replayStates[0];
		if (m.type == 'Y')
			m_movement.angle = m.prop.angle;
		else if (m.type == 'W')
			m_movement.forward = m.prop.state;
		else if (m.type == 'S')
			m_movement.backward = m.prop.state;
		else if (m.type == 'A')
			m_movement.left = m.prop.state;
		else if (m.type == 'D')
			m_movement.right = m.prop.state;
		else if (m.type == ' ')
			m_movement.jump = m.prop.state;
		else if (m.type == 'P')
			m_drift = m.prop.pos - orientation->GetPosition();
		m_replayStates.erase(m_replayStates.begin());
	}

	WVector3 correction = m_drift;// *std::min(1.0f, fDeltaTime * 4.0f);
	if (WVec3LengthSq(correction) > 0.01) {
		m_drift -= correction;
		unit->RB()->SetPosition(unit->O()->GetPosition() + correction);
	}

	RTBAI::Update(fDeltaTime);
}

void RollTheBall::RemoteControlledAI::OnNetworkUpdate(std::string prop, void* data, size_t size) {
	std::shared_ptr<WasabiGame::Unit> unit = m_unit.lock();

	if (prop == "move" && size == sizeof(RollTheBall::MOVEMENT_PACKET_STRUCT)) {
		RollTheBall::MOVEMENT_PACKET_STRUCT m;
		memcpy((void*)&m, data, sizeof(RollTheBall::MOVEMENT_PACKET_STRUCT));
		m_replayStates.push_back(m);
	} else if (prop == "pos" && size == sizeof(WVector3)) {
		WVector3 position;
		memcpy((void*)&position, data, sizeof(WVector3));

		RollTheBall::MOVEMENT_PACKET_STRUCT m;
		m.type = 'P';
		m.prop.pos = position;
		m_replayStates.push_back(m);
	}
}
