#include "RollTheBall/AI/RemoteControlledAI.hpp"


RollTheBall::RemoteControlledAI::RemoteControlledAI(std::shared_ptr<WasabiGame::Unit> unit) : RTBAI(unit) {

}

RollTheBall::RemoteControlledAI::~RemoteControlledAI() {

}

void RollTheBall::RemoteControlledAI::Update(float fDeltaTime) {
	// don't call RTBAI::Update() so that no updates are sent over the network (since this is remote-controlled only)
}
