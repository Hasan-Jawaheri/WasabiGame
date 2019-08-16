#pragma once

#include "RTBServer/Simulation/Simulation.hpp"
#include "RTBServer/Networking/Networking.hpp"
#include "RTBServer/Game/Player.hpp"

#include <cstdint>
#include <unordered_map>

namespace RTBNet {
	class RTBServerNetworking;
	class RTBServerConnectedClient;
};

class RTBGame {
	std::unordered_map<class RTBNet::RTBServerConnectedClient*, std::shared_ptr<RTBPlayer>> m_connectedPlayers;

public:
	RTBGame();

	class RTBNet::RTBServerNetworking* Networking;
	class ServerSimulation* Simulation;

	void Initialize(class RTBNet::RTBServerNetworking* networking, class ServerSimulation* simulation);
	void Destroy();

	void OnClientConnected(class RTBNet::RTBServerConnectedClient* client);
	void OnClientDisconnected(class RTBNet::RTBServerConnectedClient* client);
};
