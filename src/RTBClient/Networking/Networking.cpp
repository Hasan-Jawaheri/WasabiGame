#include "RTBClient/Networking/Networking.hpp"

RTBNet::RTBClientNetworking::RTBClientNetworking() {
	m_networkingThread = nullptr;
	m_server = new RPGNet::ServerT<RPGNet::Client>();
	m_tcpConnection = new RPGNet::ReconnectingClient(m_server);
	m_udpConnection = new RPGNet::ReconnectingClient(m_server);
	Status = RTBConnectionStatus::CONNECTION_NOT_CONNECTED;
}

void RTBNet::RTBClientNetworking::Initialize() {
	WSADATA wsa;
	(void)WSAStartup(MAKEWORD(2, 2), &wsa);

	m_server->Config.Set<int>("tcpPort", 0);
	m_server->Config.Set<int>("udpPort", 0);
	m_server->Config.Set<int>("numWorkers", 1);

	std::function<bool(HBUtils::CircularBuffer*)> onConsumeBuffer = [this](HBUtils::CircularBuffer* buffer) {
		RPGNet::NetworkUpdate update;
		size_t size = update.readPacket(buffer);
		if (size > 0) {
			auto it = m_updateCallbacks.find(update.type);
			if (it != m_updateCallbacks.end())
				it->second(update);
		}
		return true;
	};

	m_tcpConnection->SetConsumeBufferCallback(onConsumeBuffer);
	m_udpConnection->SetConsumeBufferCallback(onConsumeBuffer);

	m_networkingThread = new std::thread([this]() {
		this->m_server->Run();
	});
}

void RTBNet::RTBClientNetworking::Destroy() {
	Logout();
	m_server->Stop();
	m_networkingThread->join();
	delete m_networkingThread;
}

void RTBNet::RTBClientNetworking::Login() {
	Status = CONNECTION_CONNECTING;
	if (m_tcpConnection->Connect("127.0.0.1", 9965) == 0) {
		Status = RTBConnectionStatus::CONNECTION_CONNECTED;
		RPGNet::NetworkUpdate loginUpdate;
		RTBNet::UpdateBuilders::Login(loginUpdate, ("ghandi-" + std::to_string(rand() % 10000)).c_str(), "123456");
		SendUpdate(loginUpdate);
	}  else
		Status = RTBConnectionStatus::CONNECTION_NOT_CONNECTED;
}

void RTBNet::RTBClientNetworking::Logout() {
	m_tcpConnection->Close();
	Status = RTBConnectionStatus::CONNECTION_NOT_CONNECTED;
}

void RTBNet::RTBClientNetworking::SendUpdate(RPGNet::NetworkUpdate& update, bool important) {
	char packet[RPGNet::MAX_PACKET_SIZE];
	size_t size = update.fillPacket(packet);

	if (important)
		m_tcpConnection->Write(packet, size);
	else
		m_udpConnection->Write(packet, size);
}

void RTBNet::RTBClientNetworking::RegisterNetworkUpdateCallback(RPGNet::NetworkUpdateType type, std::function<void(RPGNet::NetworkUpdate&)> callback) {
	auto it = m_updateCallbacks.find(type);
	if (it != m_updateCallbacks.end())
		m_updateCallbacks.erase(it);
	m_updateCallbacks.insert(std::make_pair(type, callback));
}

void RTBNet::RTBClientNetworking::ClearNetworkUpdateCallback(RPGNet::NetworkUpdateType type) {
	auto it = m_updateCallbacks.find(type);
	if (it != m_updateCallbacks.end())
		m_updateCallbacks.erase(it);
}
