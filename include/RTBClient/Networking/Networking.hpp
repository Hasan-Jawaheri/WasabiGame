#pragma once

#include "WasabiGame/Networking/Data.hpp"
#include "WasabiGame/Networking/NetworkManager.hpp"
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

	class MovementNetworkingManager;

	class ClientNetworking : public WasabiGame::NetworkManager {
		std::shared_ptr<WasabiGame::NetworkListenerT<WasabiGame::NetworkClient>> m_listener;

		std::shared_ptr<WasabiGame::ReconnectingNetworkClient> m_tcpConnection;
		std::shared_ptr<WasabiGame::ReconnectingNetworkClient> m_udpConnection;

	public:
		ClientNetworking(std::shared_ptr<WasabiGame::WasabiBaseGame> app, std::shared_ptr<WasabiGame::GameConfig> config, std::shared_ptr<WasabiGame::GameScheduler> scheduler);

		std::atomic<RTBConnectionStatus> Status;
		std::shared_ptr<MovementNetworkingManager> Movement;

		virtual void Initialize() override;
		virtual void Destroy() override;
		virtual void Update(float fDeltaTime) override;

		void Login();
		void Logout();

		virtual void SendUpdate(std::shared_ptr<WasabiGame::NetworkClient> client, WasabiGame::NetworkUpdate& update, bool important = true);
		virtual void SendUpdate(uint32_t clientId, WasabiGame::NetworkUpdate& update, bool important = true);
		virtual void SendUpdate(WasabiGame::NetworkUpdate& update, bool important = true);
	};

};
