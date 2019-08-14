#include <WinSock2.h>
#include <Wasabi/Wasabi.h>

#include "RollTheBall/Networking/Server.hpp"
#include "RollTheBall/Networking/Client.hpp"
#include "RollTheBall/Networking/Data.hpp"

#include "RTBServer/Simulation/Simulation.hpp"

void RunRTBServer(bool generateAssets) {
	RPGNet::ServerT<RPGNet::Client>* server = new RPGNet::ServerT<RPGNet::Client>();

	RPGNet::ServerSimulation* simulation = new RPGNet::ServerSimulation(server, generateAssets);
	server->Scheduler.LaunchThread("simulation-thread", [server, simulation]() {
		simulation->Run();
		delete simulation;
		server->Scheduler.Stop();
	});

	server->SetOnClientConnected([](RPGNet::Selectable* _client) {
		RPGNet::Client* client = (RPGNet::Client*)_client;

		RPGNet::NetworkUpdate update;
		update.type = 0;
		update.targetId = 0;
		update.purpose = 0;
		update.dataSize = 0;

		char packet[RPGNet::MAX_PACKET_SIZE];
		size_t size = update.fillPacket(packet);
		client->Write(packet, size);
	});

	server->SetOnClientDisconnected([](RPGNet::Selectable* _client) {
		RPGNet::Client* client = (RPGNet::Client*)_client;

	});

	server->Run();
	delete server;
}