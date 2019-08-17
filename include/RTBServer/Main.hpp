#pragma once

#include "RTBServer/Simulation/Simulation.hpp"
#include "RTBServer/Networking/Networking.hpp"
#include "RTBServer/Game/RTBConnectedPlayer.hpp"

#include <cstdint>
#include <unordered_map>

namespace RTBNet {
	class RTBServerNetworking;
	class RTBServerConnectedClient;
};

class RTBServer {
	std::unordered_map<class RTBNet::RTBServerConnectedClient*, std::shared_ptr<RTBConnectedPlayer>> m_connectedPlayers;

public:
	RTBServer();

	class RTBNet::RTBServerNetworking* Networking;
	class ServerSimulation* Simulation;

	void Initialize(bool generateAssets = true);
	void Destroy();
	void Run();

	void OnClientConnected(class RTBNet::RTBServerConnectedClient* client);
	void OnClientDisconnected(class RTBNet::RTBServerConnectedClient* client);
};
