#include "Networking.hpp"

RTBNet::RTBClientNetworking::RTBClientNetworking() {
	m_server = new RPGNet::ServerT<RPGNet::Client>();
	m_tcpConnection = new RPGNet::ReconnectingClient(m_server);
	m_udpConnection = new RPGNet::ReconnectingClient(m_server);
	Status = CONNECTION_NOT_CONNECTED;
}

void RTBNet::RTBClientNetworking::Initialize() {
	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);

	m_server->Config.Set<int>("tcpPort", 0);
	m_server->Config.Set<int>("udpPort", 0);
	m_server->Config.Set<int>("numWorkers", 1);

	m_networkingThread = new std::thread([this]() {
		this->m_server->Run();
	});
}

void RTBNet::RTBClientNetworking::Destroy() {
	m_server->Stop();
	m_networkingThread->join();
	delete m_networkingThread;
}

void RTBNet::RTBClientNetworking::Login() {
	m_tcpConnection->Connect("127.0.0.1", 9987);
}

void RTBNet::RTBClientNetworking::Logout() {
	m_tcpConnection->Close();
}
