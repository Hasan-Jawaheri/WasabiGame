#include "RTBServer/Game/Game.hpp"

RTBGame::RTBGame() {
	m_networking = nullptr;
	m_simulation = nullptr;
}

void RTBGame::Initialize(RTBNet::RTBServerNetworking* networking, ServerSimulation* simulation) {
	m_networking = networking;
	m_simulation = simulation;
}

void RTBGame::Destroy() {
	m_networking = nullptr;
	m_simulation = nullptr;
}

void RTBGame::OnClientConnected(RTBNet::RTBServerConnectedClient* client) {
	Player* player = new Player();
	m_connectedPlayers.insert(std::make_pair(client, player));

	RPGNet::NetworkUpdate update;
	update.type = RTBNet::UpdateTypeEnum::UPDATE_TYPE_UNIT;
	update.targetId = 0;
	update.purpose = 0;
	update.dataSize = 0;

	char packet[RPGNet::MAX_PACKET_SIZE];
	size_t size = update.fillPacket(packet);
	client->Write(packet, size);
}

void RTBGame::OnClientDisconnected(RTBNet::RTBServerConnectedClient* client) {
	auto it = m_connectedPlayers.find(client);
	if (it != m_connectedPlayers.end()) {
		delete it->second;
		m_connectedPlayers.erase(it);
	}
}
