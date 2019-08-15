#include <WinSock2.h>
#include <Wasabi/Wasabi.h>
#include "RTBServer/Main.hpp"

#include "RollTheBall/Networking/Server.hpp"
#include "RollTheBall/Networking/Client.hpp"
#include "RollTheBall/Networking/Data.hpp"

#include "RTBServer/Networking/Networking.hpp"
#include "RTBServer/Simulation/Simulation.hpp"

void RunRTBServer(bool generateAssets) {
	RTBNet::RTBServerNetworking* networking = new RTBNet::RTBServerNetworking();
	RPGNet::Server* server = networking->GetServer();

	networking->Initialize();

	RPGNet::ServerSimulation* simulation = new RPGNet::ServerSimulation(server, generateAssets);
	server->Scheduler.LaunchThread("simulation-thread", [server, simulation]() {
		simulation->Run();
		delete simulation;
		server->Scheduler.Stop();
	});

	server->Run();
	networking->Destroy();
	delete networking;
}