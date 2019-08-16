#include "RTBServer/Simulation/Simulation.hpp"
#include "WasabiGame/Main.hpp"
#include "WasabiGame/GameStates/BaseState.hpp"

#include "WasabiGame/ResourceManager/ResourceManager.hpp"
#include "WasabiGame/Maps/MapLoader.hpp"
#include "RollTheBall/Maps/RTBMaps.hpp"
#include "RollTheBall/Units/RTBUnits.hpp"
#include "RollTheBall/AssetsGenerator/AssetsGenerator.hpp"

class SimulationWasabi : public WasabiRPG {
	friend class SimulationGameState;
	ServerSimulation* m_simulationThread;
	RTBGame* m_game;

public:
	SimulationWasabi(ServerSimulation* simulation, RTBGame* game, bool generateAssets = false) : WasabiRPG() {
		m_simulationThread = simulation;
		m_game = game;

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
};

class SimulationGameState : public BaseState {
	ServerSimulation* m_simulationThread;
	RTBGame* m_game;
	SimulationWasabi* m_wapp;

	struct {
		float fYaw, fPitch, fDist;
		WVector3 vPos;
		bool bMouseHidden;
		int lastX, lastY;
	} m_cam;

	uint32_t m_currentUnitId;
	std::mutex m_unitIdsMutex;

	std::unordered_map<std::shared_ptr<RTBPlayer>, Unit*> m_players;
	std::mutex m_playersMutex;

	uint32_t GenerateUnitId() {
		std::lock_guard lockGuard(m_unitIdsMutex);
		return m_currentUnitId++;
	}

public:
	SimulationGameState(Wasabi* app) : BaseState(app) {
		m_wapp = (SimulationWasabi*)app;
		m_simulationThread = m_wapp->m_simulationThread;
		m_game = m_wapp->m_game;

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

	void AddPlayer(std::shared_ptr<RTBPlayer> player) {
		// this is called in a different thread
		WVector3 spawnPos = WVector3(player->m_x, player->m_y, player->m_z);
		Unit* newPlayerUnit = m_wapp->Units->LoadUnit(UNIT_PLAYER, GenerateUnitId(), spawnPos);

		// special message for the player, make sure the first unit it loads is the player unit
		RPGNet::NetworkUpdate unitUnloadUpdate;
		RTBNet::UpdateBuilders::LoadUnit(unitUnloadUpdate, UNIT_PLAYER, newPlayerUnit->GetId(), spawnPos);
		m_game->Networking->SendUpdate(player->m_clientId, unitUnloadUpdate);

		{
			std::lock_guard lockGuard(m_playersMutex);
			m_players.insert(std::make_pair(player, newPlayerUnit));
		}
	}

	void RemovePlayer(std::shared_ptr<RTBPlayer> player) {
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
			m_game->Networking->SendUpdate(nullptr, unitUnloadUpdate);
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

ServerSimulation::ServerSimulation(RTBGame* game, bool generateAssets) {
	m_game = game;
	m_generateAssets = generateAssets;
	m_simulationWasabi = nullptr;
	m_gameState.store(nullptr);
	m_simulationLoaded = false;
}

void ServerSimulation::Run() {
	SimulationWasabi* wasabi = new SimulationWasabi(this, m_game, m_generateAssets);
	m_simulationWasabi = (void*)wasabi;
	RunWasabi(wasabi);
	delete wasabi;
}

void ServerSimulation::WaitForSimulationLaunch() {
	while (!m_simulationLoaded && !m_gameState.load()); // spin loop, this should only protect in the first few seconds of launch
	m_simulationLoaded = true;
}

void ServerSimulation::AddPlayer(std::shared_ptr<RTBPlayer> player) {
	// this is called in a different thread than the thread running the Wasabi instance
	WaitForSimulationLaunch();
	if (m_gameState) {
		SimulationGameState* gameState = (SimulationGameState*)m_gameState.load();
		gameState->AddPlayer(player);
	}
}

void ServerSimulation::RemovePlayer(std::shared_ptr<RTBPlayer> player) {
	// this is called in a different thread than the thread running the Wasabi instance
	WaitForSimulationLaunch();
	if (m_gameState) {
		SimulationGameState* gameState = (SimulationGameState*)m_gameState.load();
		gameState->RemovePlayer(player);
	}
}
