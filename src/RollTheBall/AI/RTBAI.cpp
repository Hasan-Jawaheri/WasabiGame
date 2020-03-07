#include "RollTheBall/AI/RTBAI.hpp"
#include "RollTheBall/Main.hpp"


RollTheBall::RTBAI::RTBAI(std::shared_ptr<WasabiGame::Unit> unit) : AI(unit) {
	m_app = unit->GetApp();
	m_updateTimer = m_app.lock().get()->Timer.GetElapsedTime();
}

RollTheBall::RTBAI::~RTBAI() {

}

void RollTheBall::RTBAI::Update(float fDeltaTime) {
	std::shared_ptr<Wasabi> app = m_app.lock();
	std::shared_ptr<WasabiGame::Unit> unit = m_unit.lock();

	// send update to server
	if (m_updateTimer + 0.05f < app->Timer.GetElapsedTime()) {
		m_updateTimer = app->Timer.GetElapsedTime();
		WVector3 rbPos = unit->O()->GetPosition();
		std::function<void(std::string, void*, uint16_t)> addProp = nullptr;
		RollTheBall::UpdateBuilders::SetUnitProps(m_unitUpdate, unit->GetId(), &addProp);
		addProp("pos", &rbPos, sizeof(WVector3));
		SendNetworkUpdate(m_unitUpdate);
	}
}

void RollTheBall::RTBAI::SendNetworkUpdate(WasabiGame::NetworkUpdate& update) {
	// Wasabi* app = m_app.lock().get();
	// ((WasabiRTB*)m_unit->GetApp())->SendNetworkUpdate(update);
}

void RollTheBall::RTBAI::OnNetworkUpdate(std::string prop, void* data, size_t size) {
	std::shared_ptr<WasabiGame::Unit> unit = m_unit.lock();
	WOrientation* orientation = unit->O();

	if (prop == "pos" && size == sizeof(WVector3)) {
		WVector3 pos;
		memcpy(&pos, data, sizeof(WVector3));
		if (orientation)
			orientation->SetPosition(pos);
	}
}
