#include "RollTheBall/AI/RemoteControlledAI.hpp"

RemoteControlledAI::RemoteControlledAI(Unit* unit) : RTBAI(unit) {

}

RemoteControlledAI::~RemoteControlledAI() {

}

void RemoteControlledAI::Update(float fDeltaTime) {
	// don't call RTBAI::Update() so that no updates are sent over the network (since this is remote-controlled only)
}
