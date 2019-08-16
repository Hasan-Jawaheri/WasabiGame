#include "RTBServer/Simulation/Simulation.hpp"

#include <Wasabi/Wasabi.h>
#include <Wasabi/Physics/Bullet/WBulletPhysics.h>
#include <Wasabi/Physics/Bullet/WBulletRigidBody.h>

#include "WasabiGame/ResourceManager/ResourceManager.hpp"
#include "WasabiGame/Maps/MapLoader.hpp"
#include "RollTheBall/Maps/RTBMaps.hpp"
#include "RollTheBall/AssetsGenerator/AssetsGenerator.hpp"

class SimulationWasabi : public Wasabi {
	ServerSimulation* m_simulationThread;
	RPGNet::Server* m_server;

	float fYaw, fPitch, fDist;
	WVector3 vPos;
	bool bMouseHidden;
	int lastX, lastY;

	std::unordered_map<uint32_t, WBulletRigidBody*> m_rigidBodies;

public:
	ResourceManager* Resources;
	MapLoader* Maps;

	SimulationWasabi(ServerSimulation* simulation, RPGNet::Server* server) : Wasabi() {
		m_simulationThread = simulation;
		m_server = server;

		fYaw = 0;
		fPitch = 30;
		fDist = -15;
		bMouseHidden = false;

		Resources = new ResourceManager(this);
		Maps = new MapLoader(this, Resources);
	}

	WError Setup() {
		SetEngineParam("appName", "RTBServer");
		SetEngineParam("enableVulkanValidation", 0);

		WError status = StartEngine(640, 480);
		if (!status) {
			return status;
		}

		PhysicsComponent = new WBulletPhysics(this);
		PhysicsComponent->Initialize();

		status = Resources->Init("Media/RollTheBall");
		if (!status) {
			WindowAndInputComponent->ShowErrorMessage("Failed to initialize the resource manager");
			return status;
		}

		SetupRTBMaps(Maps);

		Maps->SetMap(MAP_TEST);

		return status;
	}

	bool Loop(float fDeltaTime) {
		ApplyMousePivot();
		return m_simulationThread->IsRunning();
	}

	void Cleanup() {
		W_SAFE_DELETE(Maps);
		W_SAFE_DELETE(Resources);
	}

	void ApplyMousePivot() {
		WCamera* cam = CameraManager->GetDefaultCamera();
		if (WindowAndInputComponent->MouseClick(MOUSE_LEFT)) {
			if (!bMouseHidden) {
				WindowAndInputComponent->ShowCursor(false);
				bMouseHidden = true;

				lastX = WindowAndInputComponent->MouseX(MOUSEPOS_DESKTOP, 0);
				lastY = WindowAndInputComponent->MouseY(MOUSEPOS_DESKTOP, 0);

				WindowAndInputComponent->SetMousePosition(640 / 2, 480 / 2, MOUSEPOS_VIEWPORT);
			}

			int mx = WindowAndInputComponent->MouseX(MOUSEPOS_VIEWPORT, 0);
			int my = WindowAndInputComponent->MouseY(MOUSEPOS_VIEWPORT, 0);

			int dx = mx - 640 / 2;
			int dy = my - 480 / 2;

			if (fabs(dx) < 2)
				dx = 0;
			if (fabs(dy) < 2)
				dy = 0;

			fYaw += (float)dx / 2.0f;
			fPitch += (float)dy / 2.0f;

			if (dx || dy)
				WindowAndInputComponent->SetMousePosition(640 / 2, 480 / 2);
		} else {
			if (bMouseHidden) {
				WindowAndInputComponent->ShowCursor(true);
				bMouseHidden = false;

				WindowAndInputComponent->SetMousePosition(lastX, lastY, MOUSEPOS_DESKTOP);
			}
		}

		float fMouseZ = (float)WindowAndInputComponent->MouseZ();
		fDist += (fMouseZ) * (abs(fDist) / 10.0f);
		WindowAndInputComponent->SetMouseZ(0);
		fDist = fmin(-1, fDist);

		cam->SetPosition(vPos);
		cam->SetAngle(WQuaternion());
		cam->Yaw(fYaw);
		cam->Pitch(fPitch);
		cam->Move(fDist);
	}
};

ServerSimulation::ServerSimulation(RPGNet::Server* server, bool generateAssets) {
	m_server = server;
	m_generateAssets = generateAssets;
}

void ServerSimulation::Run() {
	if (m_generateAssets) {
		AssetGenerator ag("Media/RollTheBall");
		ag.Generate();
	}

	SimulationWasabi* wasabi = new SimulationWasabi(this, m_server);
	RunWasabi(wasabi);
	delete wasabi;
}
