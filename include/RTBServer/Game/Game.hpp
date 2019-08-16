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
	class RTBNet::RTBServerNetworking* m_networking;
	class ServerSimulation* m_simulation;

	std::unordered_map<class RTBNet::RTBServerConnectedClient*, Player*> m_connectedPlayers;

public:
	RTBGame();

	void Initialize(class RTBNet::RTBServerNetworking* networking, class ServerSimulation* simulation);
	void Destroy();

	void OnClientConnected(class RTBNet::RTBServerConnectedClient* client);
	void OnClientDisconnected(class RTBNet::RTBServerConnectedClient* client);
};
