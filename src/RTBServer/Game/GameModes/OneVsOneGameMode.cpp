#include "RTBServer/Game/GameModes/OneVsOneGameMode.hpp"
#include "RTBServer/Repositories/ClientsRepository.hpp"


RTBServer::GAME_MODE_MATCHMAKING_PROPERTIES RTBServer::OneVsOneGameMode::MATCHMAKING_PROPERTIES = { 1, 2 };

RTBServer::OneVsOneGameMode::OneVsOneGameMode(std::weak_ptr<ServerApplication> server) : GameMode(server) {
	m_simulation = std::make_shared<ServerSimulation>(m_server);
}

RTBServer::OneVsOneGameMode::~OneVsOneGameMode() {
}

void RTBServer::OneVsOneGameMode::Initialize(std::weak_ptr<ServerCell> weakCell) {
	std::shared_ptr<ServerApplication> server = m_server.lock();
	std::shared_ptr<ServerCell> cell = weakCell.lock();

	m_simulation->Initialize();

	ServerCellClientsMap* clientsMap = nullptr;
	server->ClientsRepository->LockCellClients(cell, &clientsMap);

	for (auto iter = clientsMap->begin(); iter != clientsMap->end(); iter++) {
		std::shared_ptr<ServerConnectedClient> client = iter->second;
		std::shared_ptr<RTBConnectedPlayer> player = std::make_shared<RTBConnectedPlayer>(client->m_id);
		m_connectedPlayers.insert(std::make_pair(client->m_id, player));

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
			this->m_simulation->AddPlayer(player);
		}
	}

	server->ClientsRepository->UnlockCellClients(cell, &clientsMap);

	std::shared_ptr<OneVsOneGameMode> sharedThis = std::dynamic_pointer_cast<OneVsOneGameMode>(shared_from_this());
	server->Scheduler->LaunchThread("SimulationThread-Cell" + cell->GetId(), [sharedThis]() {
		std::shared_ptr simulation = sharedThis->m_simulation; // acquire ownership of simulation
		while (sharedThis->m_simulation != nullptr) { // this->m_simulation is set to nullptr when the game mode exits, so exit simultion then
			simulation->Update();
		}
		simulation->Cleanup();
	});
}

void RTBServer::OneVsOneGameMode::Update() {
}

void RTBServer::OneVsOneGameMode::Cleanup() {
	m_simulation = nullptr; // this will cause simulation thread to exit gracefully
}

void RTBServer::OneVsOneGameMode::OnClientAdded(std::shared_ptr<ServerConnectedClient> client) {
	// no-op, no clients can be later on added to 1v1 cell (maybe can add spectators in the future...)
}

void RTBServer::OneVsOneGameMode::OnClientRemoved(std::shared_ptr<ServerConnectedClient> client) {
	auto it = m_connectedPlayers.find(client->m_id);
	if (it != m_connectedPlayers.end()) {
		m_simulation->RemovePlayer(it->second);
		m_connectedPlayers.erase(it); // this will remove reference to the pointer
	}
}
