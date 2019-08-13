#pragma once

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

	const size_t MAX_PACKET_SIZE = 4096;
	const size_t PACKET_META_SIZE = 10;

	typedef short NetworkUpdateType;
	typedef short PacketPurpose;

	struct NetworkUpdate {
		NetworkUpdateType type;
		PacketPurpose purpose;
		uint32_t targetId;
		uint16_t dataSize;
		char data[MAX_PACKET_SIZE - PACKET_META_SIZE];

		size_t fillPacket(char* packet);
		size_t readPacket(HBUtils::CircularBuffer* packet);
	};

	class RTBClientNetworking {
		RPGNet::ServerT<RPGNet::Client>* m_server;
		std::thread* m_networkingThread;

		RPGNet::ReconnectingClient* m_tcpConnection;
		RPGNet::ReconnectingClient* m_udpConnection;

		std::unordered_map<NetworkUpdateType, std::function<void(NetworkUpdate&)>> m_updateCallbacks;

	public:
		RTBClientNetworking();

		RTBConnectionStatus Status;

		void Initialize();
		void Destroy();

		void Login();
		void Logout();

		void SendUpdate(NetworkUpdate& update, bool important = true);
		void RegisterNetworkUpdateCallback(NetworkUpdateType type, std::function<void(NetworkUpdate&)> callback);
		void ClearNetworkUpdateCallback(NetworkUpdateType type);
	};

	enum UpdateTypeEnum {
		UPDATE_TYPE_UNIT = 1,
	};

	extern RTBClientNetworking* gRTBNetworking;
};
