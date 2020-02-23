#pragma once

#include "WasabiGame/Networking/Data.hpp"
#include "WasabiGame/Networking/NetworkListener.hpp"
#include "WasabiGame/Networking/NetworkClient.hpp"
#include "RollTheBall/Networking/Protocol.hpp"

#include <functional>
#include <unordered_map>
#include <atomic>

namespace RTBClient {

	enum RTBConnectionStatus {
		CONNECTION_NOT_CONNECTED = 0,
		CONNECTION_CONNECTING = 1,
		CONNECTION_AUTHENTICATING = 2,
		CONNECTION_CONNECTED = 3,
	};

	class ClientNetworking {
		std::shared_ptr<WasabiGame::NetworkListenerT<WasabiGame::NetworkClient>> m_listener;
		std::thread* m_networkingThread;

		std::shared_ptr<WasabiGame::ReconnectingNetworkClient> m_tcpConnection;
		std::shared_ptr<WasabiGame::ReconnectingNetworkClient> m_udpConnection;

		std::unordered_map<WasabiGame::NetworkUpdateType, std::function<void(WasabiGame::NetworkUpdate&)>> m_updateCallbacks;

	public:
		ClientNetworking(std::shared_ptr<WasabiGame::GameConfig> config, std::shared_ptr<WasabiGame::GameScheduler> scheduler);

		std::atomic<RTBConnectionStatus> Status;

		void Initialize();
		void Destroy();

		void Login();
		void Logout();

		void SendUpdate(WasabiGame::NetworkUpdate& update, bool important = true);
		void RegisterNetworkUpdateCallback(WasabiGame::NetworkUpdateType type, std::function<void(WasabiGame::NetworkUpdate&)> callback);
		void ClearNetworkUpdateCallback(WasabiGame::NetworkUpdateType type);
	};

};
