#include "RTBClient/Networking/MovementNetworkingManager.hpp"
#include "RTBClient/Networking/Networking.hpp"


RTBClient::MovementNetworkingManager::MovementNetworkingManager(std::weak_ptr<ClientNetworking> clientNetworking) {
	m_clientNetworking = clientNetworking;
}

void RTBClient::MovementNetworkingManager::Update(float fDeltaTime) {
}

void RTBClient::MovementNetworkingManager::SetYawAngle(float angle) {
	//m_clientNetworking.lock()->SendUpdate()
}

void RTBClient::MovementNetworkingManager::SetMoveForward(bool isActive) {

}

void RTBClient::MovementNetworkingManager::SetMoveBackward(bool isActive) {

}

void RTBClient::MovementNetworkingManager::SetMoveLeft(bool isActive) {

}

void RTBClient::MovementNetworkingManager::SetMoveRight(bool isActive) {

}

void RTBClient::MovementNetworkingManager::SetMoveJump(bool isActive) {

}
