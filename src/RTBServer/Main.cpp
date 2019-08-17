#include "RTBServer/Main.hpp"

RTBServer::RTBServer() {
	Networking = nullptr;
	Simulation = nullptr;
}

void RTBServer::Initialize(bool generateAssets) {
	Networking = new RTBNet::RTBServerNetworking();
	RPGNet::Server* server = Networking->GetServer();
	Simulation = new ServerSimulation(this, generateAssets);

	Networking->Initialize(this);

	server->Scheduler.LaunchThread("simulation-thread", [server, this]() {
		this->Simulation->Run();
		server->Scheduler.Stop();
	});
}

void RTBServer::Run() {
	Networking->GetServer()->Run();
}

void RTBServer::Destroy() {
	Networking->Destroy();
	Simulation->Stop();

	delete Simulation;
	delete Networking;
	Networking = nullptr;
	Simulation = nullptr;
}

void RTBServer::OnClientConnected(RTBNet::RTBServerConnectedClient* client) {
	std::shared_ptr<RTBConnectedPlayer> player = std::make_shared<RTBConnectedPlayer>(client->m_id);
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

void RTBServer::OnClientDisconnected(RTBNet::RTBServerConnectedClient* client) {
	auto it = m_connectedPlayers.find(client);
	if (it != m_connectedPlayers.end()) {
		this->Simulation->RemovePlayer(it->second);
		m_connectedPlayers.erase(it); // this will remove reference to the pointer
	}
}
