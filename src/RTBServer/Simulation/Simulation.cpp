#include "RTBServer/Simulation/Simulation.hpp"
#include "RollTheBall/Main.hpp"
#include "WasabiGame/GameStates/BaseState.hpp"

#include "WasabiGame/ResourceManager/ResourceManager.hpp"
#include "WasabiGame/Maps/MapLoader.hpp"
#include "RollTheBall/Maps/RTBMaps.hpp"
#include "RollTheBall/Units/RTBUnits.hpp"
#include "RollTheBall/AssetsGenerator/AssetsGenerator.hpp"

#include "RollTheBall/AI/RTBAI.hpp"

class SimulationWasabi : public WasabiRTB {
	friend class SimulationGameState;
	ServerSimulation* m_simulationThread;
	RTBServer* m_server;

public:
	SimulationWasabi(ServerSimulation* simulation, RTBServer* server, bool generateAssets = false) : WasabiRTB() {
		m_simulationThread = simulation;
		m_server = server;

		SetEngineParam("appName", "RTBServer");

		m_settings.debugVulkan = false;
		m_settings.debugPhysics = false;
		m_settings.fullscreen = false;
		m_settings.mediaFolder = "Media/RollTheBall";

		if (generateAssets) {
			if (!AssetGenerator(m_settings.mediaFolder).Generate())
				return;
		}
	}

	void SwitchToInitialState() {
		SetupRTBMaps(Maps);
		SetupRTBUnits(Units, true);

		PhysicsComponent->SetGravity(0, -40, 0);

		SwitchToSimulationGameState();
	}

	void SwitchToSimulationGameState();

	virtual void SendNetworkUpdate(RPGNet::NetworkUpdate& update, bool important = true) {
		m_server->Networking->SendUpdate(nullptr, update, important);
	}
};

class SimulationGameState : public BaseState {
	ServerSimulation* m_simulationThread;
	RTBServer* m_server;
	SimulationWasabi* m_wapp;

	struct {
		float fYaw, fPitch, fDist;
		WVector3 vPos;
		bool bMouseHidden;
		int lastX, lastY;
	} m_cam;

	uint32_t m_currentUnitId;
	std::mutex m_unitIdsMutex;

	std::unordered_map<std::shared_ptr<RTBConnectedPlayer>, Unit*> m_players;
	std::mutex m_playersMutex;

	uint32_t GenerateUnitId() {
		std::lock_guard lockGuard(m_unitIdsMutex);
		return m_currentUnitId++;
	}

public:
	SimulationGameState(Wasabi* app) : BaseState(app) {
		m_wapp = (SimulationWasabi*)app;
		m_simulationThread = m_wapp->m_simulationThread;
		m_server = m_wapp->m_server;

		m_currentUnitId = 1;

		m_cam.fYaw = 0;
		m_cam.fPitch = 30;
		m_cam.fDist = -15;
		m_cam.lastX = m_cam.lastY = 0;
		m_cam.bMouseHidden = false;
	}

	virtual ~SimulationGameState() {
	}

	virtual void Load() {
		m_wapp->UI->Load(m_app);

		// Load the map
		m_wapp->Maps->SetMap(MAP_TEST);

		m_server->Networking->RegisterNetworkUpdateCallback(RTBNet::UpdateTypeEnum::UPDATE_TYPE_WHOIS_UNIT, [this](RTBNet::RTBServerConnectedClient* client, RPGNet::NetworkUpdate& update) {
			uint32_t unitId;
			RTBNet::UpdateBuilders::ReadWhoIsUnitPacket(update, &unitId);
			Unit* unit = this->m_wapp->Units->GetUnit(unitId);
			if (unit && unit->O()) {
				RPGNet::NetworkUpdate unitLoadUpdate;
				RTBNet::UpdateBuilders::LoadUnit(unitLoadUpdate, unit->GetType(), unit->GetId(), unit->O()->GetPosition());
				this->m_server->Networking->SendUpdate(client->m_id, unitLoadUpdate);
			}
			return true;
		});
		
		m_server->Networking->RegisterNetworkUpdateCallback(RTBNet::UpdateTypeEnum::UPDATE_TYPE_SET_UNIT_PROPS, [this](RTBNet::RTBServerConnectedClient* client, RPGNet::NetworkUpdate& update) {
			uint32_t unitId = -1;
			Unit* unit = nullptr;
			RTBNet::UpdateBuilders::ReadSetUnitPropsPacket(update, &unitId, [this, &unitId, &unit](std::string prop, void* data, uint16_t size) {
				if (unitId == -1)
					return;
				if (!unit) {
					unit = this->m_wapp->Units->GetUnit(unitId);
					if (!unit) {
						unitId = -1;
						return;
					}
				}
				((RTBAI*)unit->GetAI())->OnNetworkUpdate(prop, data, size);
			});
			return unit != nullptr;
		});
	}

	virtual void Update(float fDeltaTime) {
		ApplyMousePivot();

		if (!m_simulationThread->IsRunning())
			m_app->SwitchState(nullptr);
	}

	virtual void Cleanup() {
		m_simulationThread->m_gameState.store(nullptr);
		((SimulationWasabi*)m_app)->Maps->SetMap(MAP_NONE);
	}

	void AddPlayer(std::shared_ptr<RTBConnectedPlayer> player) {
		// this is called in a different thread
		WVector3 spawnPos = WVector3(player->m_x, player->m_y, player->m_z);
		Unit* newPlayerUnit = m_wapp->Units->LoadUnit(UNIT_PLAYER, GenerateUnitId(), spawnPos);

		// special message for the player, make sure the first unit it loads is the player unit
		RPGNet::NetworkUpdate unitLoadUpdate;
		RTBNet::UpdateBuilders::LoadUnit(unitLoadUpdate, UNIT_PLAYER, newPlayerUnit->GetId(), spawnPos);
		m_server->Networking->SendUpdate(player->m_clientId, unitLoadUpdate);

		{
			std::lock_guard lockGuard(m_playersMutex);
			m_players.insert(std::make_pair(player, newPlayerUnit));
		}
	}

	void RemovePlayer(std::shared_ptr<RTBConnectedPlayer> player) {
		// this is called in a different thread
		Unit* playerUnit = nullptr;
		{
			std::lock_guard lockGuard(m_playersMutex);
			auto it = m_players.find(player);
			if (it != m_players.end()) {
				playerUnit = it->second;
				m_players.erase(it);
			}
		}

		if (playerUnit) {
			RPGNet::NetworkUpdate unitUnloadUpdate;
			RTBNet::UpdateBuilders::UnloadUnit(unitUnloadUpdate, playerUnit->GetId());
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

ServerSimulation::ServerSimulation(RTBServer* server, bool generateAssets) {
	m_server = server;
	m_generateAssets = generateAssets;
	m_simulationWasabi = nullptr;
	m_gameState.store(nullptr);
	m_simulationLoaded = false;
}

void ServerSimulation::Run() {
	SimulationWasabi* wasabi = new SimulationWasabi(this, m_server, m_generateAssets);
	m_simulationWasabi = (void*)wasabi;
	RunWasabi(wasabi);
	delete wasabi;
}

void ServerSimulation::WaitForSimulationLaunch() {
	while (!m_simulationLoaded && !m_gameState.load()); // spin loop, this should only protect in the first few seconds of launch
	m_simulationLoaded = true;
}

void ServerSimulation::AddPlayer(std::shared_ptr<RTBConnectedPlayer> player) {
	// this is called in a different thread than the thread running the Wasabi instance
	WaitForSimulationLaunch();
	if (m_gameState) {
		SimulationGameState* gameState = (SimulationGameState*)m_gameState.load();
		gameState->AddPlayer(player);
	}
}

void ServerSimulation::RemovePlayer(std::shared_ptr<RTBConnectedPlayer> player) {
	// this is called in a different thread than the thread running the Wasabi instance
	WaitForSimulationLaunch();
	if (m_gameState) {
		SimulationGameState* gameState = (SimulationGameState*)m_gameState.load();
		gameState->RemovePlayer(player);
	}
}
