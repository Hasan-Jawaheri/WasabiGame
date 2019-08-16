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
	RPGNet::Server* m_server;

public:
	SimulationWasabi(ServerSimulation* simulation, RPGNet::Server* server, bool generateAssets = false) : WasabiRPG() {
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
		SetupRTBUnits(Units);

		PhysicsComponent->SetGravity(0, -40, 0);

		SwitchToSimulationGameState();
	}

	void SwitchToSimulationGameState();
};

class SimulationGameState : public BaseState {
	ServerSimulation* m_simulationThread;
	RPGNet::Server* m_server;
	SimulationWasabi* m_wapp;

	float fYaw, fPitch, fDist;
	WVector3 vPos;
	bool bMouseHidden;
	int lastX, lastY;

public:
	SimulationGameState(Wasabi* app) : BaseState(app) {
		m_wapp = (SimulationWasabi*)app;
		m_simulationThread = m_wapp->m_simulationThread;
		m_server = m_wapp->m_server;

		fYaw = 0;
		fPitch = 30;
		fDist = -15;
		bMouseHidden = false;
	}

	virtual ~SimulationGameState() {
	}

	void Load() {
		m_wapp->UI->Load(m_app);

		// Load the map
		m_wapp->Maps->SetMap(MAP_TEST);
	}

	void Update(float fDeltaTime) {
		ApplyMousePivot();

		if (!m_simulationThread->IsRunning())
			m_app->SwitchState(nullptr);
	}

	void Cleanup() {
		((SimulationWasabi*)m_app)->Maps->SetMap(MAP_NONE);
	}

	void AddPlayer(RTBPlayer* player) {
	}

	void ApplyMousePivot() {
		WCamera* cam = m_app->CameraManager->GetDefaultCamera();
		if (m_app->WindowAndInputComponent->MouseClick(MOUSE_LEFT)) {
			if (!bMouseHidden) {
				m_app->WindowAndInputComponent->ShowCursor(false);
				bMouseHidden = true;

				lastX = m_app->WindowAndInputComponent->MouseX(MOUSEPOS_DESKTOP, 0);
				lastY = m_app->WindowAndInputComponent->MouseY(MOUSEPOS_DESKTOP, 0);

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

			fYaw += (float)dx / 2.0f;
			fPitch += (float)dy / 2.0f;

			if (dx || dy)
				m_app->WindowAndInputComponent->SetMousePosition(640 / 2, 480 / 2);
		} else {
			if (bMouseHidden) {
				m_app->WindowAndInputComponent->ShowCursor(true);
				bMouseHidden = false;

				m_app->WindowAndInputComponent->SetMousePosition(lastX, lastY, MOUSEPOS_DESKTOP);
			}
		}

		float fMouseZ = (float)m_app->WindowAndInputComponent->MouseZ();
		fDist += (fMouseZ) * (abs(fDist) / 10.0f);
		m_app->WindowAndInputComponent->SetMouseZ(0);
		fDist = fmin(-1, fDist);

		cam->SetPosition(vPos);
		cam->SetAngle(WQuaternion());
		cam->Yaw(fYaw);
		cam->Pitch(fPitch);
		cam->Move(fDist);
	}
};

void SimulationWasabi::SwitchToSimulationGameState() {
	SwitchState(new SimulationGameState(this));
	m_simulationThread->m_gameState = (void*)this->curState;
}

ServerSimulation::ServerSimulation(RPGNet::Server* server, bool generateAssets) {
	m_server = server;
	m_generateAssets = generateAssets;
	m_simulationWasabi = nullptr;
	m_gameState = nullptr;
}

void ServerSimulation::Run() {
	SimulationWasabi* wasabi = new SimulationWasabi(this, m_server, m_generateAssets);
	m_simulationWasabi = (void*)wasabi;
	RunWasabi(wasabi);
	delete wasabi;
}

void ServerSimulation::WaitForSimulationLaunch() {
	while (!m_gameState); // spin loop, this should only protect in the first few seconds of launch
}

void ServerSimulation::AddPlayer(RTBPlayer* player) {
	// this is called in a different thread than the thread running the Wasabi instance
	WaitForSimulationLaunch();
	SimulationGameState* gameState = (SimulationGameState*)m_gameState;
	gameState->AddPlayer(player);
}
