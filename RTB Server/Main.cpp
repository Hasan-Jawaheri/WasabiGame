#include <WinSock2.h>
#include <Wasabi.h>
#include "Lib/Server.hpp"
#include "Lib/Client.hpp"
#include "Simulation/Simulation.hpp"

void RedirectIOToConsole() {
#ifdef _WIN32
	// allocate a console for this app
	AllocConsole();

	FILE *scout, *scin;
	freopen_s(&scout, "CONOUT$", "w", stdout);
	freopen_s(&scin, "CONIN$", "r", stdin);
#endif
}

Wasabi* WInitialize() {
	RedirectIOToConsole();

	RPGNet::ServerT<RPGNet::Client> server;
	RPGNet::ServerSimulation* simulation = new RPGNet::ServerSimulation(&server);
	server.Scheduler.LaunchThread("simulation-thread", [simulation]() {
		simulation->Run();
		delete simulation;
	});
	server.Run();
	return nullptr;
}