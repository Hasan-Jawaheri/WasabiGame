#include <WinSock2.h>
#include <Wasabi/Wasabi.h>
#include "RTBServer/Main.hpp"
#include "RTBServer/Game/Game.hpp"

#include "RollTheBall/Networking/Server.hpp"
#include "RollTheBall/Networking/Client.hpp"
#include "RollTheBall/Networking/Data.hpp"

#include "RTBServer/Networking/Networking.hpp"
#include "RTBServer/Simulation/Simulation.hpp"

void RunRTBServer(bool generateAssets) {
	RTBGame* game = new RTBGame();
	RTBNet::RTBServerNetworking* networking = new RTBNet::RTBServerNetworking();
	RPGNet::Server* server = networking->GetServer();
	ServerSimulation* simulation = new ServerSimulation(game, generateAssets);

	game->Initialize(networking, simulation);
	networking->Initialize(game);

	server->Scheduler.LaunchThread("simulation-thread", [server, simulation]() {
		simulation->Run();
		delete simulation;
		server->Scheduler.Stop();
	});

	server->Run();

	networking->Destroy();
	game->Destroy();
	delete networking;
	delete game;
}