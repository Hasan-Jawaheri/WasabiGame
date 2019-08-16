#include "RTBServer/Game/Game.hpp"

RTBGame::RTBGame() {
	Networking = nullptr;
	Simulation = nullptr;
}

void RTBGame::Initialize(RTBNet::RTBServerNetworking* networking, ServerSimulation* simulation) {
	Networking = networking;
	Simulation = simulation;
}

void RTBGame::Destroy() {
	Networking = nullptr;
	Simulation = nullptr;
}

void RTBGame::OnClientConnected(RTBNet::RTBServerConnectedClient* client) {
	std::shared_ptr<RTBPlayer> player = std::make_shared<RTBPlayer>();
	m_connectedPlayers.insert(std::make_pair(client, player));

	// in a different thread, start loading player data
	Networking->GetServer()->Scheduler.SubmitWork<int>([this, client, player]() {
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
			this->Simulation->AddPlayer(player);
		}

		return 0;
	});
}

void RTBGame::OnClientDisconnected(RTBNet::RTBServerConnectedClient* client) {
	auto it = m_connectedPlayers.find(client);
	if (it != m_connectedPlayers.end()) {
		this->Simulation->RemovePlayer(it->second);
		m_connectedPlayers.erase(it); // this will remove reference to the pointer
	}
}
