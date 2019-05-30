#include "PlayerAI.hpp"
#include "../../../WasabiGame/Units/Units.hpp"

PlayerAI::PlayerAI(Unit* unit) : AI(unit) {
}

PlayerAI::~PlayerAI() {
}

void PlayerAI::Update(float fDeltaTime) {
	m_unit->O()->SetPosition(m_unit->O()->GetPosition() + WVector3(0, 0, 5) * fDeltaTime);
	WCamera* cam = APPHANDLE->CameraManager->GetDefaultCamera();
	cam->SetPosition(m_unit->O()->GetPosition()  + WVector3(0, 100, -100));
	cam->Point(m_unit->O()->GetPosition());
}

