#pragma once

#include "RTBServer/Main.hpp"

#include "WasabiGame/Networking/Data.hpp"
#include "WasabiGame/Networking/NetworkManager.hpp"
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

	class ServerNetworking : public WasabiGame::NetworkManager {
		std::shared_ptr<ServerApplication> m_app;

		std::shared_ptr<WasabiGame::NetworkListenerT<ServerConnectedClient>> m_listener;

		std::mutex m_clientsMutex;
		std::unordered_map<uint32_t, std::shared_ptr<ServerConnectedClient>> m_clients;
		uint32_t m_curClientId;

		uint32_t GenerateClientId();

	public:
		ServerNetworking(std::shared_ptr<ServerApplication> app, std::shared_ptr<WasabiGame::GameConfig> config, std::shared_ptr<WasabiGame::GameScheduler> scheduler);

		virtual void Initialize() override;
		virtual void Destroy() override;
		std::shared_ptr<WasabiGame::NetworkListenerT<ServerConnectedClient>> GetListener() const;

		virtual void SendUpdate(std::shared_ptr<WasabiGame::NetworkClient>  client, WasabiGame::NetworkUpdate& update, bool important = true) override;
		virtual void SendUpdate(uint32_t clientId, WasabiGame::NetworkUpdate& update, bool important = true) override;
		virtual void SendUpdate(WasabiGame::NetworkUpdate& update, bool important = true) override;
	};

};

