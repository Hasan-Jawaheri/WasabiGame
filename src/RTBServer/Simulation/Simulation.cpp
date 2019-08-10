#include "RTBServer/Simulation/Simulation.hpp"
#include <Wasabi/Wasabi.h>
#include <Wasabi/Physics/Bullet/WBulletPhysics.h>
#include <Wasabi/Physics/Bullet/WBulletRigidBody.h>

#include "RollTheBall/AssetsGenerator/AssetsGenerator.hpp"

class SimulationWasabi : public Wasabi {
	RPGNet::ServerSimulation* m_simulationThread;
	RPGNet::Server* m_server;

	std::unordered_map<uint32_t, WBulletRigidBody*> m_rigidBodies;

public:
	SimulationWasabi(RPGNet::ServerSimulation* simulation, RPGNet::Server* server) : Wasabi() {
		m_simulationThread = simulation;
		m_server = server;
	}

	WError Setup() {
		engineParams["enableVulkanValidation"] = 0;
		WError status = StartEngine(640, 480);
		if (!status) {
			return status;
		}

		PhysicsComponent = new WBulletPhysics(this);
		PhysicsComponent->Initialize();

		return status;
	}

	bool Loop(float fDeltaTime) {
		return m_simulationThread->IsRunning();
	}

	void Cleanup() {
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
	RunWasabi(wasabi);
	delete wasabi;
}
