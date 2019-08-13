#pragma once

#include "RollTheBall/Networking/Data.hpp"
#include "RollTheBall/Networking/Server.hpp"
#include "RollTheBall/Networking/Client.hpp"

#include <functional>
#include <unordered_map>

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

		std::unordered_map<RPGNet::NetworkUpdateType, std::function<void(RPGNet::NetworkUpdate&)>> m_updateCallbacks;

	public:
		RTBClientNetworking();

		RTBConnectionStatus Status;

		void Initialize();
		void Destroy();

		void Login();
		void Logout();

		void SendUpdate(RPGNet::NetworkUpdate& update, bool important = true);
		void RegisterNetworkUpdateCallback(RPGNet::NetworkUpdateType type, std::function<void(RPGNet::NetworkUpdate&)> callback);
		void ClearNetworkUpdateCallback(RPGNet::NetworkUpdateType type);
	};

	enum UpdateTypeEnum {
		UPDATE_TYPE_UNIT = 1,
	};
};
