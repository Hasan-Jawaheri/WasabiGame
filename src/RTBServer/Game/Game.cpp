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
	RTBPlayer* player = new RTBPlayer();
	m_connectedPlayers.insert(std::make_pair(client, player));

	// in a different thread, start loading player data
	m_networking->GetServer()->Scheduler.SubmitWork<int>([this, client, player]() {
		if (!player->Load(client->Identity)) {
			// failed to load player data
			RPGNet::NetworkUpdate update;
			RTBNet::UpdateBuilders::Error(update, "Failed to load player data");
			char packet[RPGNet::MAX_PACKET_SIZE];
			size_t size = update.fillPacket(packet);
			client->Write(packet, size);
			client->Close();
		} else {
			// add the player to the simulation
			this->m_simulation->AddPlayer(player);
		}

		return 0;
	});
}

void RTBGame::OnClientDisconnected(RTBNet::RTBServerConnectedClient* client) {
	auto it = m_connectedPlayers.find(client);
	if (it != m_connectedPlayers.end()) {
		delete it->second;
		m_connectedPlayers.erase(it);
	}
}
