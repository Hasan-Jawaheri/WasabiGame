#include "RTBClient/Networking/Networking.hpp"
#include "WasabiGame/Main.hpp"

#include <random>

RTBClient::ClientNetworking::ClientNetworking(std::shared_ptr<WasabiGame::WasabiBaseGame> app, std::shared_ptr<WasabiGame::GameConfig> config, std::shared_ptr<WasabiGame::GameScheduler> scheduler) : WasabiGame::NetworkManager() {
	m_listener = std::make_shared<WasabiGame::NetworkListenerT<WasabiGame::NetworkClient>>(config, scheduler);
	m_tcpConnection = std::make_shared<WasabiGame::ReconnectingNetworkClient>(m_listener);
	m_udpConnection = std::make_shared<WasabiGame::ReconnectingNetworkClient>(m_listener);
	Status = RTBConnectionStatus::CONNECTION_NOT_CONNECTED;
}

void RTBClient::ClientNetworking::Initialize() {
	WSADATA wsa;
	(void)WSAStartup(MAKEWORD(2, 2), &wsa);

	m_listener->Config->Set<int>("tcpPort", 0);
	m_listener->Config->Set<int>("udpPort", 0);
	m_listener->Config->Set<int>("numWorkers", 1);

	std::function<bool(WasabiGame::CircularBuffer*)> onConsumeBuffer = [this](WasabiGame::CircularBuffer* buffer) {
		WasabiGame::NetworkUpdate update;
		while (true) {
			size_t size = update.readPacket(buffer);
			if (size == 0)
				break;
			auto it = m_updateCallbacks.find(update.type);
			if (it != m_updateCallbacks.end())
				it->second(nullptr, update);
		}
		return true;
	};

	m_tcpConnection->SetConsumeBufferCallback(onConsumeBuffer);
	m_udpConnection->SetConsumeBufferCallback(onConsumeBuffer);
	
	m_tcpConnection->SetOnConnectedCallback([this]() {
		this->Status = RTBConnectionStatus::CONNECTION_CONNECTED;
		// send a login update
		WasabiGame::NetworkUpdate loginUpdate;
		RollTheBall::UpdateBuilders::Login(loginUpdate, ("ghandi-" + std::to_string(std::rand() % 10000)).c_str(), "123456");
		SendUpdate(loginUpdate);
	});

	m_tcpConnection->SetOnDisconnectedCallback([this]() {
		this->Status = RTBConnectionStatus::CONNECTION_NOT_CONNECTED;
	});

	m_tcpConnection->SetOnConnectingCallback([this]() {
		this->Status = RTBConnectionStatus::CONNECTION_CONNECTING;
	});

	m_tcpConnection->SetOnConnectionFailedCallback([this]() {
		this->Status = RTBConnectionStatus::CONNECTION_NOT_CONNECTED;
	});

	m_listener->RunDetached();
}

void RTBClient::ClientNetworking::Destroy() {
	Logout();
	m_listener->SetOnClientConnected(nullptr);
	m_listener->SetOnClientDisconnected(nullptr);
	m_listener->Stop();
}

void RTBClient::ClientNetworking::Update(float fDeltaTime) {
}

void RTBClient::ClientNetworking::Login() {
	m_tcpConnection->Connect("127.0.0.1", 9965);
}

void RTBClient::ClientNetworking::Logout() {
	m_tcpConnection->StopReconnecting();
}

void RTBClient::ClientNetworking::SendUpdate(std::shared_ptr<WasabiGame::NetworkClient> client, WasabiGame::NetworkUpdate& update, bool important) {
	SendUpdate(update, important);
}

void RTBClient::ClientNetworking::SendUpdate(uint32_t clientId, WasabiGame::NetworkUpdate& update, bool important) {
	SendUpdate(update, important);
}

void RTBClient::ClientNetworking::SendUpdate(WasabiGame::NetworkUpdate& update, bool important) {
	char packet[WasabiGame::MAX_PACKET_SIZE];
	size_t size = update.fillPacket(packet);

	if (important)
		m_tcpConnection->Write(packet, size);
	else
		m_udpConnection->Write(packet, size);
}
