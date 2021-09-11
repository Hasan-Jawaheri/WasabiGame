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
	m_gameStateSync = std::make_unique<GameStateSyncBackend>(sharedServer->Networking, &m_wasabi->Timer);
	m_targetUpdateDurationMs = 1000.0 / sharedServer->Config->Get<float>("MaxSimulationUpdatesPerSecond");
	m_millisecondsSleptBeyondTarget = 0.0;

	m_wasabi->Resources = std::make_shared<WasabiGame::ResourceManager>(m_wasabi, true);
	m_wasabi->Maps = std::make_shared<WasabiGame::MapLoader>(m_wasabi, m_wasabi->Resources);
	m_wasabi->Units = std::make_shared<WasabiGame::UnitsManager>(m_wasabi, m_wasabi->Resources);
	m_wasabi->Networking = sharedServer->Networking; // we have a separate networking class in ServerApplication, use that
	m_currentUnitId = 1;
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
	m_gameStateSync->Update(deltaTime);
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

	// special message for the player, make sure the first unit it loads is the player unit
	WasabiGame::NetworkUpdate update;
	RollTheBall::UpdateBuilders::LoadUnit(update, static_cast<uint32_t>(RollTheBall::UNIT_TYPE::UNIT_PLAYER), newPlayerUnit->GetId(), spawnPos);
	m_wasabi->Networking->SendUpdate(player->m_clientId, update);

	// tell the player to load the map
	RollTheBall::UpdateBuilders::LoadMap(update, static_cast<uint32_t>(m_map));
	m_wasabi->Networking->SendUpdate(player->m_clientId, update);

	m_gameStateSync->AddPlayer(player, newPlayerUnit);
}

void RTBServer::ServerSimulation::RemovePlayer(std::shared_ptr<RTBServer::RTBConnectedPlayer> player) {
	//
	// this is called in a different thread than Initialize(), Cleanup() and Update()
	//

	std::shared_ptr<WasabiGame::Unit> playerUnit = m_gameStateSync->RemovePlayer(player->m_clientId);

	if (playerUnit) {
		WasabiGame::NetworkUpdate unitUnloadUpdate;
		RollTheBall::UpdateBuilders::UnloadUnit(unitUnloadUpdate, playerUnit->GetId());
		m_wasabi->Networking->SendUpdate(nullptr, unitUnloadUpdate);
		m_wasabi->Units->DestroyUnit(playerUnit);
	}
}

bool RTBServer::ServerSimulation::OnReceivedNetworkUpdate(std::shared_ptr<RTBConnectedPlayer> client, WasabiGame::NetworkUpdate update) {
	//
	// this is called in a different thread than Initialize(), Cleanup() and Update()
	//

	switch (static_cast<RollTheBall::NetworkUpdateTypeEnum>(update.type)) {
	case RollTheBall::NetworkUpdateTypeEnum::UPDATE_TYPE_WHOIS_UNIT:
	{
		uint32_t unitId;
		RollTheBall::UpdateBuilders::ReadWhoIsUnitPacket(update, &unitId);
		std::shared_ptr<WasabiGame::Unit> unit = m_wasabi->Units->GetUnit(unitId);
		if (unit && unit->O()) {
			WasabiGame::NetworkUpdate unitLoadUpdate;
			RollTheBall::UNIT_TYPE unitType = static_cast<RollTheBall::UNIT_TYPE>(unit->GetType());
			if (unitType == RollTheBall::UNIT_TYPE::UNIT_PLAYER && unitId != 0)
				unitType = RollTheBall::UNIT_TYPE::UNIT_OTHER_PLAYER;
			RollTheBall::UpdateBuilders::LoadUnit(unitLoadUpdate, static_cast<uint32_t>(unitType), unitId, unit->O()->GetPosition());
			m_wasabi->Networking->SendUpdate(client->m_clientId, unitLoadUpdate);
		}
		return true;
	}
	case RollTheBall::NetworkUpdateTypeEnum::UPDATE_TYPE_SET_UNIT_PROPS:
	{
		uint32_t unitId = -1;
		uint32_t clientId = client->m_clientId;
		std::shared_ptr<WasabiGame::Unit> unit = nullptr;
		RollTheBall::UpdateBuilders::ReadSetUnitPropsPacket(update, &unitId,
			[this, clientId, &unitId, &unit](std::string prop, void* data, uint16_t size) {
				if (unitId == -1)
					return;
				if (!unit) {
					unit = m_wasabi->Units->GetUnit(unitId);
					if (!unit) {
						unitId = -1;
						return;
					}
				}
				std::dynamic_pointer_cast<RollTheBall::RTBAI>(unit->GetAI())->OnNetworkUpdate(prop, data, size);
			}
		);
		return unit != nullptr;
	}
	case RollTheBall::NetworkUpdateTypeEnum::UPDATE_TYPE_SET_PLAYER_INPUT:
		return true;
	}

	return false;
}
