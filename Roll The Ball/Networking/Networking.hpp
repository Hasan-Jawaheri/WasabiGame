#pragma once

#include "../../RTB Server/Lib/Server.hpp"
#include "../../RTB Server/Lib/Client.hpp"

namespace RTBNet {

	enum RTBConnectionStatus {
		CONNECTION_NOT_CONNECTED = 0,
		CONNECTION_CONNECTING = 1,
		CONNECTION_AUTHENTICATING = 2,
		CONNECTION_CONNECTED = 3,
	};

	class RTBClientNetworking {
		RPGNet::ServerT<RPGNet::Client>* m_server;
		std::thread* m_networkingThread;

		RPGNet::ReconnectingClient* m_tcpConnection;
		RPGNet::ReconnectingClient* m_udpConnection;

	public:
		RTBClientNetworking();

		RTBConnectionStatus Status;

		void Initialize();
		void Destroy();

		void Login();
		void Logout();
	};

};

extern RTBNet::RTBClientNetworking* gRTBNetworking;
