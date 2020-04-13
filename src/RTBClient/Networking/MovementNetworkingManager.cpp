#include "RTBClient/Networking/MovementNetworkingManager.hpp"
#include "RTBClient/Networking/Networking.hpp"
#include "WasabiGame/Main.hpp"
#include "RollTheBall/AI/RemoteControlledAI.hpp"


RTBClient::MovementNetworkingManager::MovementNetworkingManager(std::weak_ptr<ClientNetworking> clientNetworking) {
	m_clientNetworking = clientNetworking;
	m_periodicUpdateTimer = m_clientNetworking.lock()->GetApp()->Timer.GetElapsedTime();
}

void RTBClient::MovementNetworkingManager::Update(float fDeltaTime) {
	std::shared_ptr<WasabiGame::WasabiBaseGame> app = m_clientNetworking.lock()->GetApp();
	// send periodic update to server
	if (m_periodicUpdateTimer + 0.5f < app->Timer.GetElapsedTime()) {
		m_periodicUpdateTimer = app->Timer.GetElapsedTime();

		std::shared_ptr<ClientNetworking> clientNetworking = m_clientNetworking.lock();
		std::function<void(std::string, void*, uint16_t)> setProp;
		RollTheBall::UpdateBuilders::SetUnitProps(m_update, 0, &setProp);
		RollTheBall::MOVEMENT_PACKET_STRUCT m;
		m.type = 'P';
		m.time = clientNetworking->GetApp()->Timer.GetElapsedTime();
		m.prop.pos = m_playerPosition;
		setProp("move", (void*)&m, sizeof(RollTheBall::MOVEMENT_PACKET_STRUCT));
		clientNetworking->SendUpdate(m_update);
	}
}

void RTBClient::MovementNetworkingManager::UpdatePlayer(std::shared_ptr<WasabiGame::Unit> player) {
	m_playerPosition = player->O()->GetPosition();
}

void RTBClient::MovementNetworkingManager::SetYawAngle(float angle) {
	std::shared_ptr<ClientNetworking> clientNetworking = m_clientNetworking.lock();
	std::function<void(std::string, void*, uint16_t)> setProp;
	RollTheBall::UpdateBuilders::SetUnitProps(m_update, 0, &setProp);
	RollTheBall::MOVEMENT_PACKET_STRUCT m;
	m.type = 'Y';
	m.time = clientNetworking->GetApp()->Timer.GetElapsedTime();
	m.prop.angle = angle;
	setProp("move", (void*)&m, sizeof(RollTheBall::MOVEMENT_PACKET_STRUCT));
	clientNetworking->SendUpdate(m_update);
}

void RTBClient::MovementNetworkingManager::SetMoveForward(bool isActive) {
	std::shared_ptr<ClientNetworking> clientNetworking = m_clientNetworking.lock();
	std::function<void(std::string, void*, uint16_t)> setProp;
	RollTheBall::UpdateBuilders::SetUnitProps(m_update, 0, &setProp);
	RollTheBall::MOVEMENT_PACKET_STRUCT m;
	m.type = 'W';
	m.time = clientNetworking->GetApp()->Timer.GetElapsedTime();
	m.prop.state = isActive;
	setProp("move", (void*)&m, sizeof(RollTheBall::MOVEMENT_PACKET_STRUCT));
	clientNetworking->SendUpdate(m_update);
}

void RTBClient::MovementNetworkingManager::SetMoveBackward(bool isActive) {
	std::shared_ptr<ClientNetworking> clientNetworking = m_clientNetworking.lock();
	std::function<void(std::string, void*, uint16_t)> setProp;
	RollTheBall::UpdateBuilders::SetUnitProps(m_update, 0, &setProp);
	RollTheBall::MOVEMENT_PACKET_STRUCT m;
	m.type = 'S';
	m.time = clientNetworking->GetApp()->Timer.GetElapsedTime();
	m.prop.state = isActive;
	setProp("move", (void*)&m, sizeof(RollTheBall::MOVEMENT_PACKET_STRUCT));
	clientNetworking->SendUpdate(m_update);
}

void RTBClient::MovementNetworkingManager::SetMoveLeft(bool isActive) {
	std::shared_ptr<ClientNetworking> clientNetworking = m_clientNetworking.lock();
	std::function<void(std::string, void*, uint16_t)> setProp;
	RollTheBall::UpdateBuilders::SetUnitProps(m_update, 0, &setProp);
	RollTheBall::MOVEMENT_PACKET_STRUCT m;
	m.type = 'A';
	m.time = clientNetworking->GetApp()->Timer.GetElapsedTime();
	m.prop.state = isActive;
	setProp("move", (void*)&m, sizeof(RollTheBall::MOVEMENT_PACKET_STRUCT));
	clientNetworking->SendUpdate(m_update);
}

void RTBClient::MovementNetworkingManager::SetMoveRight(bool isActive) {
	std::shared_ptr<ClientNetworking> clientNetworking = m_clientNetworking.lock();
	std::function<void(std::string, void*, uint16_t)> setProp;
	RollTheBall::UpdateBuilders::SetUnitProps(m_update, 0, &setProp);
	RollTheBall::MOVEMENT_PACKET_STRUCT m;
	m.type = 'D';
	m.time = clientNetworking->GetApp()->Timer.GetElapsedTime();
	m.prop.state = isActive;
	setProp("move", (void*)&m, sizeof(RollTheBall::MOVEMENT_PACKET_STRUCT));
	clientNetworking->SendUpdate(m_update);
}

void RTBClient::MovementNetworkingManager::SetMoveJump(bool isActive) {
	std::shared_ptr<ClientNetworking> clientNetworking = m_clientNetworking.lock();
	std::function<void(std::string, void*, uint16_t)> setProp;
	RollTheBall::UpdateBuilders::SetUnitProps(m_update, 0, &setProp);
	RollTheBall::MOVEMENT_PACKET_STRUCT m;
	m.type = ' ';
	m.time = clientNetworking->GetApp()->Timer.GetElapsedTime();
	m.prop.state = isActive;
	setProp("move", (void*)&m, sizeof(RollTheBall::MOVEMENT_PACKET_STRUCT));
	clientNetworking->SendUpdate(m_update);
}
