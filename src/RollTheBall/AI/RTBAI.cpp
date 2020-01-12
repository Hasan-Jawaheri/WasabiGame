#include "RollTheBall/AI/RTBAI.hpp"
#include "RollTheBall/Main.hpp"

RTBAI::RTBAI(Unit* unit) : AI(unit) {
	m_app = unit->GetApp();
	m_updateTimer = m_app->Timer.GetElapsedTime();
}

RTBAI::~RTBAI() {

}

void RTBAI::Update(float fDeltaTime) {
	// send update to server
	if (m_updateTimer + 0.05f < m_app->Timer.GetElapsedTime()) {
		m_updateTimer = m_app->Timer.GetElapsedTime();
		WVector3 rbPos = m_unit->O()->GetPosition();
		std::function<void(std::string, void*, uint16_t)> addProp = nullptr;
		RTBNet::UpdateBuilders::SetUnitProps(m_unitUpdate, m_unit->GetId(), &addProp);
		addProp("pos", &rbPos, sizeof(WVector3));
		SendNetworkUpdate(m_unitUpdate);
	}
}

void RTBAI::SendNetworkUpdate(RPGNet::NetworkUpdate& update) {
	((WasabiRTB*)m_unit->GetApp())->SendNetworkUpdate(update);
}

void RTBAI::OnNetworkUpdate(std::string prop, void* data, size_t size) {
	WOrientation* orientation = m_unit->O();

	if (prop == "pos" && size == sizeof(WVector3)) {
		WVector3 pos;
		memcpy(&pos, data, sizeof(WVector3));
		if (orientation)
			orientation->SetPosition(pos);
	}
}
