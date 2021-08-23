#include "RTBClient/Networking/MovementNetworkingManager.hpp"
#include "RTBClient/Networking/Networking.hpp"
#include "RollTheBall/AI/RemoteControlledAI.hpp"
#include "WasabiGame/Main.hpp"


RTBClient::MovementNetworkingManager::MovementNetworkingManager(std::weak_ptr<ClientNetworking> clientNetworking, ::WTimer* clientTimer) {
	m_clientTimer = clientTimer;
	m_clientNetworking = clientNetworking;
	m_periodicUpdateTimer = m_clientTimer->GetElapsedTime();
}

void RTBClient::MovementNetworkingManager::Update(float fDeltaTime) {
	// send periodic update to server
	if (m_periodicUpdateTimer + 0.5f < m_clientTimer->GetElapsedTime()) {
		m_periodicUpdateTimer = m_clientTimer->GetElapsedTime();

		std::shared_ptr<ClientNetworking> clientNetworking = m_clientNetworking.lock();
		std::function<void(std::string, void*, uint16_t)> setProp;
		RollTheBall::UpdateBuilders::SetUnitProps(m_update, 0, &setProp);
		RollTheBall::MOVEMENT_PACKET_STRUCT m;
		m.type = 'P';
		m.time = m_clientTimer->GetElapsedTime();
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
	m.time = m_clientTimer->GetElapsedTime();
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
	m.time = m_clientTimer->GetElapsedTime();
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
	m.time = m_clientTimer->GetElapsedTime();
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
	m.time = m_clientTimer->GetElapsedTime();
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
	m.time = m_clientTimer->GetElapsedTime();
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
	m.time = m_clientTimer->GetElapsedTime();
	m.prop.state = isActive;
	setProp("move", (void*)&m, sizeof(RollTheBall::MOVEMENT_PACKET_STRUCT));
	clientNetworking->SendUpdate(m_update);
}
