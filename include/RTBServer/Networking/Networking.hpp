#pragma once

#include "RTBServer/Main.hpp"
#include "RTBServer/Game/Game.hpp"

#include "RollTheBall/Networking/Data.hpp"
#include "RollTheBall/Networking/Server.hpp"
#include "RollTheBall/Networking/Client.hpp"
#include "RollTheBall/Networking/Protocol.hpp"

#include <mutex>

class RTBGame;

namespace RTBNet {

	class RTBServerConnectedClient : public RPGNet::Client {
	public:
		RTBServerConnectedClient(class RPGNet::Server* server, SOCKET sock, struct sockaddr_in addr)
			: RPGNet::Client(server, sock, addr), m_id(0) {}

		uint32_t m_id;
	};

	class RTBServerNetworking {
		RTBGame* m_game;
		RPGNet::ServerT<RTBServerConnectedClient>* m_server;
		std::unordered_map<RPGNet::NetworkUpdateType, std::function<void(RPGNet::NetworkUpdate&)>> m_updateCallbacks;

		std::mutex m_clientsMutex;
		std::unordered_map<uint32_t, RTBServerConnectedClient*> m_clients;
		uint32_t m_curClientId;

		uint32_t GenerateClientId();

	public:
		RTBServerNetworking();

		void Initialize(RTBGame* game);
		void Destroy();
		RPGNet::ServerT<RTBServerConnectedClient>* GetServer() const;

		void SendUpdate(RTBServerConnectedClient*  client, RPGNet::NetworkUpdate& update, bool important = true);
		void SendUpdate(uint32_t clientId, RPGNet::NetworkUpdate& update, bool important = true);

		void RegisterNetworkUpdateCallback(RPGNet::NetworkUpdateType type, std::function<void(RPGNet::NetworkUpdate&)> callback);
		void ClearNetworkUpdateCallback(RPGNet::NetworkUpdateType type);
	};

};

