#include "RTBServer/Simulation/Simulation.hpp"

#include <Wasabi/Wasabi.h>
#include <Wasabi/Physics/Bullet/WBulletPhysics.h>
#include <Wasabi/Physics/Bullet/WBulletRigidBody.h>

#include "WasabiGame/ResourceManager/ResourceManager.hpp"
#include "WasabiGame/Maps/MapLoader.hpp"
#include "RollTheBall/Maps/RTBMaps.hpp"
#include "RollTheBall/AssetsGenerator/AssetsGenerator.hpp"

class SimulationWasabi : public Wasabi {
	RPGNet::ServerSimulation* m_simulationThread;
	RPGNet::Server* m_server;

	float fYaw, fPitch, fDist;
	WVector3 vPos;

	std::unordered_map<uint32_t, WBulletRigidBody*> m_rigidBodies;

public:
	SimulationWasabi(RPGNet::ServerSimulation* simulation, RPGNet::Server* server) : Wasabi() {
		m_simulationThread = simulation;
		m_server = server;

		fYaw = 0;
		fPitch = 30;
		fDist = -15;
	}

	WError Setup() {
		engineParams["enableVulkanValidation"] = 0;
		WError status = StartEngine(640, 480);
		if (!status) {
			return status;
		}

		PhysicsComponent = new WBulletPhysics(this);
		PhysicsComponent->Initialize();

		status = ResourceManager::Init("Media/RollTheBall");
		if (!status) {
			WindowAndInputComponent->ShowErrorMessage("Failed to initialize the resource manager");
			return status;
		}

		SetupRTBMaps();

		MapLoader::SetMap(MAP_TEST);

		return status;
	}

	bool Loop(float fDeltaTime) {
		ApplyMousePivot();
		return m_simulationThread->IsRunning();
	}

	void Cleanup() {
	}


	void ApplyMousePivot() {
		WCamera* cam = CameraManager->GetDefaultCamera();
		static bool bMouseHidden = false;
		static int lx, ly;
		if (WindowAndInputComponent->MouseClick(MOUSE_LEFT)) {
			if (!bMouseHidden) {
				WindowAndInputComponent->ShowCursor(false);
				bMouseHidden = true;

				lx = WindowAndInputComponent->MouseX(MOUSEPOS_DESKTOP, 0);
				ly = WindowAndInputComponent->MouseY(MOUSEPOS_DESKTOP, 0);

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

				WindowAndInputComponent->SetMousePosition(lx, ly, MOUSEPOS_DESKTOP);
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

RPGNet::ServerSimulation::ServerSimulation(Server* server) {
	m_server = server;
}

void RPGNet::ServerSimulation::Run() {
	{
		AssetGenerator ag("Media/RollTheBall");
		ag.Generate();
	}

	SimulationWasabi* wasabi = new SimulationWasabi(this, m_server);
	APPHANDLE = wasabi;
	RunWasabi(wasabi);
	delete wasabi;
}
