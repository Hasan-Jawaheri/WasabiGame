#include "RTBServer/Main.hpp"

RTBServer::ServerApplication::ServerApplication() : std::enable_shared_from_this<ServerApplication>() {
	Config = std::make_shared<WasabiGame::GameConfig>();
	Scheduler = std::make_shared<WasabiGame::GameScheduler>();
	Networking = nullptr;
	Simulation = nullptr;
}

void RTBServer::ServerApplication::Initialize(bool generateAssets) {
	Networking = std::make_shared<ServerNetworking>(Config, Scheduler);
	Simulation = std::make_shared<ServerSimulation>(shared_from_this(), generateAssets);
	std::shared_ptr<WasabiGame::NetworkListenerT<ServerConnectedClient>> listener = Networking->GetListener();

	Networking->Initialize(shared_from_this());

	Scheduler->LaunchThread("simulation-thread", [this]() {
		this->Simulation->Run();
		this->Scheduler->Stop();
	});
}

void RTBServer::ServerApplication::Run() {
	Networking->GetListener()->Run();
}

void RTBServer::ServerApplication::Destroy() {
	Networking->Destroy();
	Simulation->Stop();
	Scheduler->Stop();

	Networking.reset();
	Simulation.reset();
	Scheduler.reset();
	Config.reset();
}

void RTBServer::ServerApplication::OnClientConnected(std::shared_ptr<RTBServer::ServerConnectedClient> client) {
	std::shared_ptr<RTBConnectedPlayer> player = std::make_shared<RTBConnectedPlayer>(client->m_id);
	m_connectedPlayers.insert(std::make_pair(client->m_id, player));

	// in a different thread, start loading player data
	Scheduler->SubmitWork<int>([this, client, player]() {
		if (!player->Load(client->Identity)) {
			// failed to load player data
			WasabiGame::NetworkUpdate update;
			RollTheBall::UpdateBuilders::Error(update, "Failed to load player data");
			char packet[WasabiGame::MAX_PACKET_SIZE];
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

void RTBServer::ServerApplication::OnClientDisconnected(std::shared_ptr<RTBServer::ServerConnectedClient> client) {
	auto it = m_connectedPlayers.find(client->m_id);
	if (it != m_connectedPlayers.end()) {
		this->Simulation->RemovePlayer(it->second);
		m_connectedPlayers.erase(it); // this will remove reference to the pointer
	}
}
