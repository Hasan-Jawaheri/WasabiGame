#include "RTBServer/Simulation/Simulation.hpp"
#include "RollTheBall/Main.hpp"
#include "WasabiGame/GameStates/BaseState.hpp"

#include "WasabiGame/ResourceManager/ResourceManager.hpp"
#include "WasabiGame/Maps/MapLoader.hpp"
#include "RollTheBall/Maps/RTBMaps.hpp"
#include "RollTheBall/Units/RTBUnits.hpp"
#include "RollTheBall/AssetsGenerator/AssetsGenerator.hpp"

#include "RollTheBall/AI/RTBAI.hpp"


class SimulationWasabi : public WasabiGame::WasabiBaseGame {
	friend class SimulationGameState;
	std::shared_ptr<RTBServer::ServerSimulation> m_simulationThread;
	std::shared_ptr<RTBServer::ServerApplication> m_server;

public:
	SimulationWasabi(std::shared_ptr<RTBServer::ServerSimulation> simulation, std::shared_ptr<RTBServer::ServerApplication> server, bool generateAssets = false) : WasabiGame::WasabiBaseGame() {
		m_simulationThread = simulation;
		m_server = server;

		SetEngineParam("appName", "RTBServer");

		m_settings.debugVulkan = false;
		m_settings.debugPhysics = false;
		m_settings.fullscreen = false;
		m_settings.mediaFolder = "Media/RollTheBall";

		if (generateAssets) {
			if (!RollTheBall::AssetGenerator(m_settings.mediaFolder).Generate())
				return;
		}
	}

	void SwitchToInitialState() {
		RollTheBall::SetupRTBMaps(Maps);
		RollTheBall::SetupRTBUnits(Units, true);

		PhysicsComponent->SetGravity(0, -40, 0);

		SwitchToSimulationGameState();
	}

	void SwitchToSimulationGameState();

	//virtual void SendNetworkUpdate(WasabiGame::NetworkUpdate& update, bool important = true) override {
	//	m_server->Networking->SendUpdate(nullptr, update, important);
	//}
};

class SimulationGameState : public WasabiGame::BaseGameState {
	std::shared_ptr<RTBServer::ServerSimulation> m_simulationThread;
	std::shared_ptr<RTBServer::ServerApplication> m_server;
	SimulationWasabi* m_wapp;

	struct {
		float fYaw, fPitch, fDist;
		WVector3 vPos;
		bool bMouseHidden;
		int lastX, lastY;
	} m_cam;

	uint32_t m_currentUnitId;
	std::mutex m_unitIdsMutex;

	std::unordered_map<uint32_t, std::pair<std::shared_ptr<RTBServer::RTBConnectedPlayer>, std::shared_ptr<WasabiGame::Unit>>> m_players;
	std::mutex m_playersMutex;
	float m_lastBroadcastTime;

	uint32_t GenerateUnitId() {
		std::lock_guard lockGuard(m_unitIdsMutex);
		return m_currentUnitId++;
	}

public:
	SimulationGameState(Wasabi* app) : WasabiGame::BaseGameState(app) {
		m_wapp = (SimulationWasabi*)app;
		m_simulationThread = m_wapp->m_simulationThread;
		m_server = m_wapp->m_server;

		m_currentUnitId = 1;
		m_lastBroadcastTime = 0.0f;

		m_cam.fYaw = 0;
		m_cam.fPitch = 30;
		m_cam.fDist = -15;
		m_cam.lastX = m_cam.lastY = 0;
		m_cam.bMouseHidden = false;
	}

	virtual ~SimulationGameState() {
	}

	virtual void Load() {
		m_wapp->UI->Load();

		// Load the map
		m_wapp->Maps->SetMap(RollTheBall::MAP_TEST);

		m_server->Networking->RegisterNetworkUpdateCallback(RollTheBall::NetworkUpdateTypeEnum::UPDATE_TYPE_WHOIS_UNIT, [this](std::shared_ptr<RTBServer::ServerConnectedClient> client, WasabiGame::NetworkUpdate& update) {
			uint32_t unitId;
			RollTheBall::UpdateBuilders::ReadWhoIsUnitPacket(update, &unitId);
			uint32_t actualUnitId = unitId;
			if (unitId == 0) {
				std::lock_guard lockGuard(m_playersMutex);
				actualUnitId = m_players.find(client->m_id)->second.second->GetId();
			}
			std::shared_ptr<WasabiGame::Unit> unit = this->m_wapp->Units->GetUnit(actualUnitId);
			if (unit && unit->O()) {
				WasabiGame::NetworkUpdate unitLoadUpdate;
				uint32_t unitType = unit->GetType();
				if (unitType == RollTheBall::UNIT_PLAYER && unitId != 0)
					unitType = RollTheBall::UNIT_OTHER_PLAYER;
				RollTheBall::UpdateBuilders::LoadUnit(unitLoadUpdate, unitType, unitId, unit->O()->GetPosition());
				this->m_server->Networking->SendUpdate(client->m_id, unitLoadUpdate);
			}
			return true;
		});
		
		m_server->Networking->RegisterNetworkUpdateCallback(RollTheBall::NetworkUpdateTypeEnum::UPDATE_TYPE_SET_UNIT_PROPS, [this](std::shared_ptr<RTBServer::ServerConnectedClient> client, WasabiGame::NetworkUpdate& update) {
			uint32_t unitId = -1;
			uint32_t clientId = client->m_id;
			std::shared_ptr<WasabiGame::Unit> unit = nullptr;
			RollTheBall::UpdateBuilders::ReadSetUnitPropsPacket(update, &unitId, [this, clientId, &unitId, &unit](std::string prop, void* data, uint16_t size) {
				if (unitId == -1)
					return;
				if (unitId == 0) {
					// players think their id is 0
					std::lock_guard lockGuard(m_playersMutex);
					unitId = m_players.find(clientId)->second.second->GetId();
				}
				if (!unit) {
					unit = this->m_wapp->Units->GetUnit(unitId);
					if (!unit) {
						unitId = -1;
						return;
					}
				}
				std::dynamic_pointer_cast<RollTheBall::RTBAI>(unit->GetAI())->OnNetworkUpdate(prop, data, size);
			});
			return unit != nullptr;
		});
	}

	virtual void Update(float fDeltaTime) {
		ApplyMousePivot();

		if (!m_simulationThread->IsRunning())
			m_app->SwitchState(nullptr);

		{
			std::lock_guard lockGuard(m_playersMutex);
			if (m_app->Timer.GetElapsedTime() - m_lastBroadcastTime > 0.05f) {
				m_lastBroadcastTime = m_app->Timer.GetElapsedTime();
				for (auto senderPlayer : m_players) {
					WasabiGame::NetworkUpdate update;
					std::function<void(std::string, void*, uint16_t)> addProp = nullptr;
					RollTheBall::UpdateBuilders::SetUnitProps(update, senderPlayer.second.second->GetId(), &addProp);
					WVector3 pos = senderPlayer.second.second->O()->GetPosition();
					addProp("pos", (void*)&pos, sizeof(WVector3));

					for (auto receiverPlayer : m_players) {
						if (senderPlayer.second != receiverPlayer.second) {
							m_server->Networking->SendUpdate(receiverPlayer.second.first->m_clientId, update);
						}
					}
				}
			}
		}
	}

	virtual void Cleanup() {
		m_simulationThread->m_gameState.store(nullptr);
		((SimulationWasabi*)m_app)->Maps->SetMap(RollTheBall::MAP_NONE);
	}

	void AddPlayer(std::shared_ptr<RTBServer::RTBConnectedPlayer> player) {
		// this is called in a different thread
		WVector3 spawnPos = WVector3(player->m_x, player->m_y, player->m_z);
		std::shared_ptr<WasabiGame::Unit> newPlayerUnit = m_wapp->Units->LoadUnit(RollTheBall::UNIT_PLAYER, GenerateUnitId(), spawnPos);

		// special message for the player, make sure the first unit it loads is the player unit (and each player thinks his id is 0)
		WasabiGame::NetworkUpdate unitLoadUpdate;
		RollTheBall::UpdateBuilders::LoadUnit(unitLoadUpdate, RollTheBall::UNIT_PLAYER, 0, spawnPos);
		m_server->Networking->SendUpdate(player->m_clientId, unitLoadUpdate);

		{
			std::lock_guard lockGuard(m_playersMutex);
			m_players.insert(std::make_pair(player->m_clientId, std::make_pair(player, newPlayerUnit)));
		}
	}

	void RemovePlayer(std::shared_ptr<RTBServer::RTBConnectedPlayer> player) {
		// this is called in a different thread
		std::shared_ptr<WasabiGame::Unit> playerUnit = nullptr;
		{
			std::lock_guard lockGuard(m_playersMutex);
			auto it = m_players.find(player->m_clientId);
			if (it != m_players.end()) {
				playerUnit = it->second.second;
				m_players.erase(it);
			}
		}

		if (playerUnit) {
			WasabiGame::NetworkUpdate unitUnloadUpdate;
			RollTheBall::UpdateBuilders::UnloadUnit(unitUnloadUpdate, playerUnit->GetId());
			m_server->Networking->SendUpdate(nullptr, unitUnloadUpdate);
			m_wapp->Units->DestroyUnit(playerUnit);
		}
	}

	void ApplyMousePivot() {
		WCamera* cam = m_app->CameraManager->GetDefaultCamera();
		if (m_app->WindowAndInputComponent->MouseClick(MOUSE_LEFT)) {
			if (!m_cam.bMouseHidden) {
				m_app->WindowAndInputComponent->ShowCursor(false);
				m_cam.bMouseHidden = true;

				m_cam.lastX = m_app->WindowAndInputComponent->MouseX(MOUSEPOS_DESKTOP, 0);
				m_cam.lastY = m_app->WindowAndInputComponent->MouseY(MOUSEPOS_DESKTOP, 0);

				m_app->WindowAndInputComponent->SetMousePosition(640 / 2, 480 / 2, MOUSEPOS_VIEWPORT);
			}

			int mx = m_app->WindowAndInputComponent->MouseX(MOUSEPOS_VIEWPORT, 0);
			int my = m_app->WindowAndInputComponent->MouseY(MOUSEPOS_VIEWPORT, 0);

			int dx = mx - 640 / 2;
			int dy = my - 480 / 2;

			if (fabs(dx) < 2)
				dx = 0;
			if (fabs(dy) < 2)
				dy = 0;

			m_cam.fYaw += (float)dx / 2.0f;
			m_cam.fPitch += (float)dy / 2.0f;

			if (dx || dy)
				m_app->WindowAndInputComponent->SetMousePosition(640 / 2, 480 / 2);
		} else {
			if (m_cam.bMouseHidden) {
				m_app->WindowAndInputComponent->ShowCursor(true);
				m_cam.bMouseHidden = false;

				m_app->WindowAndInputComponent->SetMousePosition(m_cam.lastX, m_cam.lastY, MOUSEPOS_DESKTOP);
			}
		}

		float fMouseZ = (float)m_app->WindowAndInputComponent->MouseZ();
		m_cam.fDist += (fMouseZ) * (abs(m_cam.fDist) / 10.0f);
		m_app->WindowAndInputComponent->SetMouseZ(0);
		m_cam.fDist = fmin(-1, m_cam.fDist);

		cam->SetPosition(m_cam.vPos);
		cam->SetAngle(WQuaternion());
		cam->Yaw(m_cam.fYaw);
		cam->Pitch(m_cam.fPitch);
		cam->Move(m_cam.fDist);
	}
};

void SimulationWasabi::SwitchToSimulationGameState() {
	SwitchState(new SimulationGameState(this));
	m_simulationThread->m_gameState.store((void*)this->curState);
}

RTBServer::ServerSimulation::ServerSimulation(std::shared_ptr<ServerApplication> server, bool generateAssets) : std::enable_shared_from_this<ServerSimulation>() {
	m_server = server;
	m_generateAssets = generateAssets;
	m_gameState.store(nullptr);
	m_simulationLoaded = false;
}

void RTBServer::ServerSimulation::Run() {
	std::shared_ptr<SimulationWasabi> wasabi = std::make_shared<SimulationWasabi>(shared_from_this(), m_server, m_generateAssets);
	RunWasabi(wasabi.get());
	wasabi.reset();
}

void RTBServer::ServerSimulation::WaitForSimulationLaunch() {
	while (!m_simulationLoaded && !m_gameState.load()); // spin loop, this should only protect in the first few seconds of launch
	m_simulationLoaded = true;
}

void RTBServer::ServerSimulation::AddPlayer(std::shared_ptr<RTBConnectedPlayer> player) {
	// this is called in a different thread than the thread running the Wasabi instance
	WaitForSimulationLaunch();
	if (m_gameState) {
		SimulationGameState* gameState = (SimulationGameState*)m_gameState.load();
		gameState->AddPlayer(player);
	}
}

void RTBServer::ServerSimulation::RemovePlayer(std::shared_ptr<RTBConnectedPlayer> player) {
	// this is called in a different thread than the thread running the Wasabi instance
	WaitForSimulationLaunch();
	if (m_gameState) {
		SimulationGameState* gameState = (SimulationGameState*)m_gameState.load();
		gameState->RemovePlayer(player);
	}
}
