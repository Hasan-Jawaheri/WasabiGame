#include "RollTheBall/AI/RTBAI.hpp"

RTBAI::RTBAI(Unit* unit) : AI(unit) {

}

RTBAI::~RTBAI() {

}

void RTBAI::Update(float fDeltaTime) {

}

void RTBAI::SentNetworkUpdate(RPGNet::NetworkUpdate& update) {

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
