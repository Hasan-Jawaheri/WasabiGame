#include "RTBServer/Simulation/Simulation.hpp"
#include "RollTheBall/Main.hpp"
#include "WasabiGame/Main.hpp"
#include "Wasabi/Physics/Bullet/WBulletPhysics.hpp"

#include "WasabiGame/ResourceManager/ResourceManager.hpp"
#include "WasabiGame/Maps/MapLoader.hpp"
#include "RollTheBall/Maps/RTBMaps.hpp"
#include "RollTheBall/Units/RTBUnits.hpp"
#include "RollTheBall/AssetsGenerator/AssetsGenerator.hpp"

#include "RollTheBall/AI/RTBAI.hpp"

class PhysicsOnlyWasabi : public WasabiGame::WasabiBaseGame {
public:
	PhysicsOnlyWasabi() {
		PhysicsComponent = new WBulletPhysics(this);
		FileManager = new WFileManager(this);
	}

	virtual WError Setup() override { return WError(); }
	virtual bool Loop(float fDeltaTime) override { return false;  }
	virtual void Cleanup() override {}
	virtual void SwitchToInitialState() override {}
};


RTBServer::ServerSimulation::ServerSimulation(std::weak_ptr<ServerApplication> server) {
	std::shared_ptr<ServerApplication> sharedServer = server.lock();

	m_server = server;
	m_wasabi = std::make_shared<PhysicsOnlyWasabi>();
	m_targetUpdateDurationMs = 1000.0 / sharedServer->Config->Get<float>("MaxSimulationUpdatesPerSecond");
	m_millisecondsSleptBeyondTarget = 0.0;

	m_wasabi->Resources = std::make_shared<WasabiGame::ResourceManager>(m_wasabi, true);
	m_wasabi->Maps = std::make_shared<WasabiGame::MapLoader>(m_wasabi, m_wasabi->Resources);
	m_wasabi->Units = std::make_shared<WasabiGame::UnitsManager>(m_wasabi, m_wasabi->Resources);
	m_wasabi->Networking = sharedServer->Networking; // we have a separate networking class in ServerApplication, use that
	m_currentUnitId = 1;
	m_lastBroadcastTime = 0.0f;
	m_map = RollTheBall::MAP_NAME::MAP_NONE;
}

RTBServer::ServerSimulation::~ServerSimulation() {
}

uint32_t RTBServer::ServerSimulation::GenerateUnitId() {
	std::scoped_lock lockGuard(m_unitIdsMutex);
	return m_currentUnitId++;
}

void RTBServer::ServerSimulation::Initialize(RollTheBall::MAP_NAME map) {
	//
	// This is called in a different thread than Update() and Cleanup()
	//

	std::shared_ptr<ServerApplication> server = m_server.lock();

	m_wasabi->Timer.Start();
	m_wasabi->PhysicsComponent->Initialize(true);
	m_wasabi->PhysicsComponent->SetGravity(0, -40, 0);
	m_wasabi->PhysicsComponent->Start();

	m_wasabi->Resources->Init(server->Config->Get<std::string>("MediaFolderNoTrailingSlash"));
	RollTheBall::SetupRTBUnits(m_wasabi->Units, true);
	RollTheBall::SetupRTBMaps(m_wasabi->Maps);

	m_map = map;
	m_wasabi->Maps->SetMap(static_cast<uint32_t>(m_map));

	std::shared_ptr<ServerNetworking> Networking = std::static_pointer_cast<ServerNetworking>(m_wasabi->Networking);

	Networking->RegisterNetworkUpdateCallback(static_cast<WasabiGame::NetworkUpdateType>(RollTheBall::NetworkUpdateTypeEnum::UPDATE_TYPE_WHOIS_UNIT),
		[this](std::shared_ptr<WasabiGame::Selectable> _client, WasabiGame::NetworkUpdate& update) {
			std::shared_ptr<RTBServer::ServerConnectedClient> client = std::static_pointer_cast<RTBServer::ServerConnectedClient>(_client);
			uint32_t unitId;
			RollTheBall::UpdateBuilders::ReadWhoIsUnitPacket(update, &unitId);
			uint32_t actualUnitId = unitId;
			if (unitId == 0) {
				std::scoped_lock lockGuard(m_playersMutex);
				actualUnitId = m_players.find(client->m_id)->second.second->GetId();
			}
			std::shared_ptr<WasabiGame::Unit> unit = this->m_wasabi->Units->GetUnit(actualUnitId);
			if (unit && unit->O()) {
				WasabiGame::NetworkUpdate unitLoadUpdate;
				RollTheBall::UNIT_TYPE unitType = static_cast<RollTheBall::UNIT_TYPE>(unit->GetType());
				if (unitType == RollTheBall::UNIT_TYPE::UNIT_PLAYER && unitId != 0)
					unitType = RollTheBall::UNIT_TYPE::UNIT_OTHER_PLAYER;
				RollTheBall::UpdateBuilders::LoadUnit(unitLoadUpdate, static_cast<uint32_t>(unitType), unitId, unit->O()->GetPosition());
				this->m_wasabi->Networking->SendUpdate(client->m_id, unitLoadUpdate);
			}
			return true;
		}
	);
	
	Networking->RegisterNetworkUpdateCallback(static_cast<WasabiGame::NetworkUpdateType>(RollTheBall::NetworkUpdateTypeEnum::UPDATE_TYPE_SET_UNIT_PROPS),
		[this](std::shared_ptr<WasabiGame::Selectable> _client, WasabiGame::NetworkUpdate& update) {
			std::shared_ptr<RTBServer::ServerConnectedClient> client = std::static_pointer_cast<RTBServer::ServerConnectedClient>(_client);
			uint32_t unitId = -1;
			uint32_t clientId = client->m_id;
			std::shared_ptr<WasabiGame::Unit> unit = nullptr;
			RollTheBall::UpdateBuilders::ReadSetUnitPropsPacket(update, &unitId, [this, clientId, &unitId, &unit](std::string prop, void* data, uint16_t size) {
				if (unitId == -1)
					return;
				if (unitId == 0) {
					// players think their id is 0
					std::scoped_lock lockGuard(m_playersMutex);
					unitId = m_players.find(clientId)->second.second->GetId();
				}
				if (!unit) {
					unit = this->m_wasabi->Units->GetUnit(unitId);
					if (!unit) {
						unitId = -1;
						return;
					}
				}
				std::dynamic_pointer_cast<RollTheBall::RTBAI>(unit->GetAI())->OnNetworkUpdate(prop, data, size);
			});
			return unit != nullptr;
		}
	);

	InitializeFPSRegulation();
}

void RTBServer::ServerSimulation::InitializeFPSRegulation() {
	m_lastUpdateStart = std::chrono::high_resolution_clock::now();
}

double RTBServer::ServerSimulation::RegulateFPS() {
	m_wasabi->Timer.GetElapsedTime(true); // all subsequent calls to GetElapsedTime will get the time recorded here (for consistent and performance)
	auto lastUpdateEnd = std::chrono::high_resolution_clock::now();
	double updateDurationMs = std::chrono::duration<double, std::milli>(lastUpdateEnd - m_lastUpdateStart).count();

	double targetSleepTimeMs = std::max(0.0, m_targetUpdateDurationMs - updateDurationMs);
	double targetSleepTimeWithErrorMs = targetSleepTimeMs - m_millisecondsSleptBeyondTarget / 2.0;

	if (targetSleepTimeWithErrorMs >= 5) {
		// sleep for targetSleepTimeWithErrorMs at 1ms chunks, when we are 5ms away, stop sleeping and just hot loop to avoid sleep inaccuracy
		double totalSleptDuration = 0;
		do {
			if (totalSleptDuration > targetSleepTimeWithErrorMs - 5)
				std::this_thread::sleep_for(std::chrono::duration<double, std::milli>(1));
			totalSleptDuration = std::chrono::duration<double, std::milli>(std::chrono::high_resolution_clock::now() - lastUpdateEnd).count();
		} while (totalSleptDuration < targetSleepTimeWithErrorMs);

		lastUpdateEnd = std::chrono::high_resolution_clock::now();
		updateDurationMs = std::chrono::duration<double, std::milli>(lastUpdateEnd - m_lastUpdateStart).count();

		/* 
		 * correct m_millisecondsSleptBeyondTarget: we slept for m_millisecondsSleptBeyondTarget/2 less than target, so error now is
		 * m_millisecondsSleptBeyondTarget/2 millis less. But due to sleep inaccuracy, there may still be error. The error
		 * is now just the sleep inaccuracy
		 */
		double sleepInaccuracyMs = std::min(100.0, std::max(-100.0, updateDurationMs - m_targetUpdateDurationMs)); // +-100ms max
		m_millisecondsSleptBeyondTarget -= m_millisecondsSleptBeyondTarget / 2.0 - sleepInaccuracyMs;
	} else {
		/*
		 * m_millisecondsSleptBeyondTarget is too big (i.e. we are behind schedule) and we skipped sleeping this frame. But because
		 * we skipped sleeping, we are doing better than target so correct m_millisecondsSleptBeyondTarget by how much better we did
		 * than target (that's how much ahead of schedule we've gotten)
		 */
		m_millisecondsSleptBeyondTarget -= targetSleepTimeMs;
	}

	m_lastUpdateStart = std::chrono::high_resolution_clock::now();
	return (m_targetUpdateDurationMs + m_millisecondsSleptBeyondTarget) / 1000.0;
}

void RTBServer::ServerSimulation::Update() {
	//
	// This is called in a different thread than Initialize() and Cleanup()
	//

	double deltaTime = RegulateFPS();
	m_wasabi->PhysicsComponent->Step(deltaTime);

	m_wasabi->Maps->Update(deltaTime);
	m_wasabi->Units->Update(deltaTime);
	m_wasabi->Resources->Update(deltaTime);

	{
		std::scoped_lock lockGuard(m_playersMutex);
		if (m_wasabi->Timer.GetElapsedTime() - m_lastBroadcastTime > 0.05f) {
			m_lastBroadcastTime = m_wasabi->Timer.GetElapsedTime();
			for (auto senderPlayer : m_players) {
				WVector3 pos = senderPlayer.second.second->O()->GetPosition();
				WasabiGame::NetworkUpdate updateToOtherPlayers, updateToPlayer;
				std::function<void(std::string, void*, uint16_t)> addProp = nullptr;
				RollTheBall::UpdateBuilders::SetUnitProps(updateToOtherPlayers, senderPlayer.second.second->GetId(), &addProp);
				addProp("pos", (void*)&pos, sizeof(WVector3));
				RollTheBall::UpdateBuilders::SetUnitProps(updateToPlayer, 0, &addProp);
				addProp("pos", (void*)&pos, sizeof(WVector3));

				for (auto receiverPlayer : m_players) {
					if (senderPlayer.second != receiverPlayer.second) {
						m_wasabi->Networking->SendUpdate(receiverPlayer.second.first->m_clientId, updateToOtherPlayers, false);
					} else {
						// tell each player where he actually is, every player thinks their unit id is 0
						m_wasabi->Networking->SendUpdate(receiverPlayer.second.first->m_clientId, updateToPlayer, false);
					}
				}
			}
		}
	}
}

void RTBServer::ServerSimulation::Cleanup() {
	//
	// This is called in a different thread than Update() and Initialize()
	//

	m_wasabi->PhysicsComponent->Cleanup();
}

void RTBServer::ServerSimulation::AddPlayer(std::shared_ptr<RTBServer::RTBConnectedPlayer> player) {
	//
	// this is called in a different thread than Initialize(), Cleanup() and Update()
	//
	WVector3 spawnPos = WVector3(player->m_x, player->m_y, player->m_z);
	std::shared_ptr<WasabiGame::Unit> newPlayerUnit = m_wasabi->Units->LoadUnit(static_cast<uint32_t>(RollTheBall::UNIT_TYPE::UNIT_PLAYER), GenerateUnitId(), spawnPos);

	// special message for the player, make sure the first unit it loads is the player unit (and each player thinks his id is 0)
	WasabiGame::NetworkUpdate update;
	RollTheBall::UpdateBuilders::LoadUnit(update, static_cast<uint32_t>(RollTheBall::UNIT_TYPE::UNIT_PLAYER), 0, spawnPos);
	m_wasabi->Networking->SendUpdate(player->m_clientId, update);

	// tell the player to load the map
	RollTheBall::UpdateBuilders::LoadMap(update, static_cast<uint32_t>(m_map));
	m_wasabi->Networking->SendUpdate(player->m_clientId, update);

	{
		std::scoped_lock lockGuard(m_playersMutex);
		m_players.insert(std::make_pair(player->m_clientId, std::make_pair(player, newPlayerUnit)));
	}
}

void RTBServer::ServerSimulation::RemovePlayer(std::shared_ptr<RTBServer::RTBConnectedPlayer> player) {
	//
	// this is called in a different thread than Initialize(), Cleanup() and Update()
	//

	std::shared_ptr<WasabiGame::Unit> playerUnit = nullptr;
	{
		std::scoped_lock lockGuard(m_playersMutex);
		auto it = m_players.find(player->m_clientId);
		if (it != m_players.end()) {
			playerUnit = it->second.second;
			m_players.erase(it);
		}
	}

	if (playerUnit) {
		WasabiGame::NetworkUpdate unitUnloadUpdate;
		RollTheBall::UpdateBuilders::UnloadUnit(unitUnloadUpdate, playerUnit->GetId());
		m_wasabi->Networking->SendUpdate(nullptr, unitUnloadUpdate);
		m_wasabi->Units->DestroyUnit(playerUnit);
	}
}
