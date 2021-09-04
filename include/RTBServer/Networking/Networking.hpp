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
			: WasabiGame::NetworkClient(listener, sock, addr), m_id(0), m_udpAddrLen(0) {
			memset(Identity.accountName, 0, sizeof(Identity.accountName));
		}

		WasabiGame::ClientIdentity Identity;
		uint32_t m_id;
		struct sockaddr m_udpAddr;
		int m_udpAddrLen;
	};

	class ServerNetworking : public WasabiGame::NetworkManager {
		std::shared_ptr<ServerApplication> m_app;

		std::shared_ptr<WasabiGame::NetworkListenerT<ServerConnectedClient>> m_listener;
		std::shared_ptr<WasabiGame::UDPServerSideNetworkClient> m_UDPClient;

		std::mutex m_clientsMutex;
		std::unordered_map<uint32_t, std::shared_ptr<ServerConnectedClient>> m_clients;
		std::unordered_map<uint32_t, uint32_t> m_udpPortToClientId;
		uint32_t m_curClientId;

		uint32_t GenerateClientId();
		void OnClientConnected(std::shared_ptr<WasabiGame::Selectable> client);
		void OnClientDisconnected(std::shared_ptr<WasabiGame::Selectable> client);
		bool OnReceivedNetworkUpdate(std::shared_ptr<ServerConnectedClient> client, WasabiGame::NetworkUpdate update);
		void OnReceivedUDPPacket(void* packet, size_t length, sockaddr* addrFrom, int addrLen);

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

