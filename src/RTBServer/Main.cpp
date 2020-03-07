#include "RTBServer/Main.hpp"
#include "RTBServer/Simulation/Simulation.hpp"

#include "RollTheBall/AssetsGenerator/AssetsGenerator.hpp"

RTBServer::ServerApplication::ServerApplication(bool generateAssets, bool enableVulkanDebugging, bool enablePhysicsDebugging) : WasabiGame::WasabiBaseGame() {
	SetEngineParam("appName", "RTBServer");
	
#ifdef _DEBUG
	m_settings.debugVulkan = enableVulkanDebugging;
	m_settings.debugPhysics = enablePhysicsDebugging;
	m_settings.fullscreen = false;
#else
	m_settings.debugVulkan = false;
	m_settings.debugPhysics = false;
	m_settings.fullscreen = false;
#endif

	m_settings.mediaFolder = "Media/RollTheBall";

	if (generateAssets) {
		if (!RollTheBall::AssetGenerator(m_settings.mediaFolder).Generate())
			return;
	}
}

RTBServer::ServerApplication::~ServerApplication() {
}

void RTBServer::ServerApplication::SwitchToInitialState() {
	Networking = std::make_shared<RTBServer::ServerNetworking>(shared_from_this(), Config, Scheduler);
	Networking->Initialize();

	Simulation = std::make_shared<ServerSimulationGameState>(this);
	SwitchState(Simulation.get());
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
