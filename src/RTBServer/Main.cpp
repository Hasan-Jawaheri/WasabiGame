#include "RTBServer/Main.hpp"

#include "RTBServer/Cells/LoginCell.hpp"
#include "RTBServer/Cells/MatchmakingCell.hpp"

#include "RTBServer/Repositories/ClientsRepository.hpp"

#include "RTBServer/Simulation/Simulation.hpp"

#include "RollTheBall/AssetsGenerator/AssetsGenerator.hpp"
#include "RollTheBall/Maps/RTBMaps.hpp"
#include "RollTheBall/Units/RTBUnits.hpp"

RTBServer::ServerApplication::ServerApplication(bool generateAssets, bool enableVulkanDebugging, bool enablePhysicsDebugging) : WasabiGame::WasabiBaseGame() {
	SetEngineParam("appName", "RTBServer");
	
	m_settings.debugVulkan = enableVulkanDebugging;
	m_settings.debugPhysics = enablePhysicsDebugging;
	m_settings.fullscreen = false;

	m_settings.mediaFolder = "Media/RollTheBall";

	if (generateAssets) {
		if (!RollTheBall::AssetGenerator(m_settings.mediaFolder).Generate())
			return;
	}

	Config->Set("MatchmakingCellUpdatePeriodS", 1.0f);
}

RTBServer::ServerApplication::~ServerApplication() {
}

void RTBServer::ServerApplication::SwitchToInitialState() {
	std::shared_ptr<ServerApplication> sharedThis = std::static_pointer_cast<ServerApplication>(shared_from_this());

	ClientsRepository = std::make_shared<RTBServer::ServerClientsRepository>();

	Networking = std::make_shared<RTBServer::ServerNetworking>(std::static_pointer_cast<WasabiBaseGame>(sharedThis), Config, Scheduler);
	Networking->Initialize();

	RegisterCell(std::make_shared<LoginCell>(sharedThis));
	RegisterCell(std::make_shared<MatchmakingCell>(sharedThis));

	Scheduler->LaunchThread("ServerCellRunner", [this]() { this->UpdateCellsThread(); });

	RollTheBall::SetupRTBMaps(Maps);
	RollTheBall::SetupRTBUnits(Units, true);

	Simulation = std::make_shared<ServerSimulationGameState>(this);
	SwitchState(Simulation.get());
}

void RTBServer::ServerApplication::UpdateCellsThread() {
	//
	// This is running in a separate thread
	//
	while (true) {
		std::lock_guard<std::mutex> lock(m_cellsMutex);
		for (int i = 0; i < m_cells.size(); i++) {
			if (!m_cells[i]->Update()) {
				m_cells.erase(m_cells.begin() + i);
				i--;
			}
		}
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
}

/*void RTBServer::ServerApplication::OnClientConnected(std::shared_ptr<RTBServer::ServerConnectedClient> client) {
	// send any new connection to the login cell
	GetLoginCell()->AddClient(client);

	// std::shared_ptr<RTBConnectedPlayer> player = std::make_shared<RTBConnectedPlayer>(client->m_id);
	// m_connectedPlayers.insert(std::make_pair(client->m_id, player));

	// // in a different thread, start loading player data
	// Scheduler->SubmitWork<int>([this, client, player]() {
	// 	if (!player->Load(client->Identity)) {
	// 		// failed to load player data
	// 		WasabiGame::NetworkUpdate update;
	// 		RollTheBall::UpdateBuilders::Error(update, "Failed to load player data");
	// 		char packet[WasabiGame::MAX_PACKET_SIZE];
	// 		size_t size = update.fillPacket(packet);
	// 		client->Write(packet, size);
	// 		client->Close();
	// 	} else {
	// 		// add the player to the simulation
	// 		this->Simulation->AddPlayer(player);
	// 	}

	// 	return 0;
	// });
}

void RTBServer::ServerApplication::OnClientDisconnected(std::shared_ptr<RTBServer::ServerConnectedClient> client) {
	// @TODO: IMPLEMENT
	// auto it = m_connectedPlayers.find(client->m_id);
	// if (it != m_connectedPlayers.end()) {
	// 	this->Simulation->RemovePlayer(it->second);
	// 	m_connectedPlayers.erase(it); // this will remove reference to the pointer
	// }
}*/

void RTBServer::ServerApplication::RegisterCell(std::shared_ptr<ServerCell> cell) {
	std::lock_guard<std::mutex> lock(m_cellsMutex);
	m_cells.push_back(cell);
}

std::shared_ptr<RTBServer::ServerCell> RTBServer::ServerApplication::GetLoginCell() const {
	return m_cells[0];
}

std::shared_ptr<RTBServer::ServerCell> RTBServer::ServerApplication::GetMatchmakingCell() const {
	return m_cells[1];
}
