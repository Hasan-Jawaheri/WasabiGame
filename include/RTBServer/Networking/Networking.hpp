#pragma once

#include "RTBServer/Main.hpp"

#include "WasabiGame/Networking/Data.hpp"
#include "WasabiGame/Networking/NetworkClient.hpp"
#include "WasabiGame/Networking/NetworkListener.hpp"
#include "RollTheBall/Networking/Protocol.hpp"

#include <mutex>


namespace RTBServer {

	class ServerApplication;

	class ServerConnectedClient : public WasabiGame::NetworkClient {
	public:
		ServerConnectedClient(std::shared_ptr<WasabiGame::NetworkListener> listener, SOCKET sock, struct sockaddr_in addr)
			: WasabiGame::NetworkClient(listener, sock, addr), m_id(0) {
			memset(Identity.accountName, 0, sizeof(Identity.accountName));
		}

		WasabiGame::ClientIdentity Identity;
		uint32_t m_id;
	};

	class ServerNetworking {
		std::shared_ptr<ServerApplication> m_app;
		std::shared_ptr<WasabiGame::NetworkListenerT<ServerConnectedClient>> m_listener;
		std::unordered_map<WasabiGame::NetworkUpdateType, std::function<bool(std::shared_ptr<ServerConnectedClient>, WasabiGame::NetworkUpdate&)>> m_updateCallbacks;

		std::mutex m_clientsMutex;
		std::unordered_map<uint32_t, std::shared_ptr<ServerConnectedClient>> m_clients;
		uint32_t m_curClientId;

		uint32_t GenerateClientId();

	public:
		ServerNetworking(std::shared_ptr<WasabiGame::GameConfig> config, std::shared_ptr<WasabiGame::GameScheduler> scheduler);

		void Initialize(std::shared_ptr<ServerApplication> app);
		void Destroy();
		std::shared_ptr<WasabiGame::NetworkListenerT<ServerConnectedClient>> GetListener() const;

		void SendUpdate(std::shared_ptr<ServerConnectedClient>  client, WasabiGame::NetworkUpdate& update, bool important = true);
		void SendUpdate(uint32_t clientId, WasabiGame::NetworkUpdate& update, bool important = true);

		void RegisterNetworkUpdateCallback(WasabiGame::NetworkUpdateType type, std::function<bool(std::shared_ptr<ServerConnectedClient>, WasabiGame::NetworkUpdate&)> callback);
		void ClearNetworkUpdateCallback(WasabiGame::NetworkUpdateType type);

		bool Authenticate(WasabiGame::ClientIdentity& identity);
	};

};

