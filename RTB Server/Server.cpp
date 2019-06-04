#include "Server.hpp"

#include <WS2tcpip.h>

RPGNet::Server::Server(CreateClientFunction createClient) : Config(), Scheduler() {
	m_isRunning = true;
	m_createClient = createClient;
	m_TCPServer = { 0 };
	m_UDPServer = { 0 };
}

RPGNet::Server::~Server() {
}

void RPGNet::Server::Run() {
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		// LOG(fatal) << "Failed to initialize WinSock.";
		return;
	}

	m_TCPServer.port = 9987; Config.Get<int>("tcpPort");
	m_UDPServer.port = 9986; Config.Get<int>("udpPort");

	strcpy_s(m_TCPServer.host, sizeof(m_TCPServer.host), Config.Get<char*>("hostname"));
	strcpy_s(m_UDPServer.host, sizeof(m_UDPServer.host), Config.Get<char*>("hostname"));

	if (m_TCPServer.Initialize() == 0) {
		if (m_UDPServer.Initialize() == 0) {
			Scheduler.LaunchWorkers(8);// Config.Get<int>("numWorkers");
			Scheduler.LaunchThread("selectables-loop", [this]() {
				this->SelectablesLoop();
				this->Scheduler.Stop();
			});
			Scheduler.LaunchThread("asdf", []() {
				std::this_thread::sleep_for(std::chrono::milliseconds(200));

				SOCKET s = socket(AF_INET, SOCK_STREAM, 0);
				struct sockaddr_in addr;
				addr.sin_port = htons(9987);
				addr.sin_family = AF_INET;
				const char* hostname = "127.0.0.1";
				inet_pton(AF_INET, hostname, &addr.sin_addr);
				int r1 = connect(s, (struct sockaddr*) & addr, sizeof(addr));
				int r2 = send(s, "HALO", 4, 0);
				closesocket(s);
			});
			Scheduler.Run();

			m_UDPServer.Cleanup();
		}
		m_TCPServer.Cleanup();
	}

	WSACleanup();
}

void RPGNet::Server::Stop() {
	m_isRunning = false;
}

void RPGNet::Server::SelectablesLoop() {
	std::vector<Selectable*> clientsToDelete;
	while (m_isRunning && m_TCPServer.IsAlive() && m_UDPServer.IsAlive()) {
		fd_set readFDs, writeFDs;
		int maxFDs = max(m_TCPServer.sock, m_UDPServer.sock);
		FD_ZERO(&readFDs);
		FD_ZERO(&writeFDs);
		FD_SET(m_TCPServer.sock, &readFDs);
		FD_SET(m_UDPServer.sock, &readFDs);

		for (auto client : m_clients) {
			maxFDs = max(maxFDs, client.first->fd());
			FD_SET(client.first->fd(), &readFDs);
			if (client.first->HasPendingWrites())
				FD_SET(client.first->fd(), &writeFDs);
		}

		select(maxFDs, &readFDs, &writeFDs, NULL, NULL);

		if (FD_ISSET(m_TCPServer.sock, &readFDs)) {
			// pending TCP connection
			Selectable* newClient = m_TCPServer.AcceptConnection(this);
			if (newClient)
				m_clients.insert(std::make_pair(newClient, newClient->fd()));
		}
		if (FD_ISSET(m_UDPServer.sock, &readFDs)) {
			// pending UDP packet
			m_UDPServer.ReadPacket();
		}

		for (auto client : m_clients) {
			if (FD_ISSET(client.second, &readFDs)) {
				if (!client.first->OnReadReady()) {
					clientsToDelete.push_back(client.first);
				}
			}
			if (FD_ISSET(client.second, &writeFDs)) {
				if (!client.first->OnWriteReady()) {
					clientsToDelete.push_back(client.first);
				}
			}
		}

		for (auto client : clientsToDelete) {
			m_clients.erase(client);
			delete client;
		}
		clientsToDelete.clear();
	}
}

void RPGNet::Server::NotifyWriteAvailable() {
	// @TODO: implement (write to an fd that select always wants to read from, and dump data away)
}

int RPGNet::Server::TCPServer::Initialize() {
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
		//LOG(fatal) << "Failed to create TCP server socket (" << WSAGetLastError() << ").";
		return 1;
	}

	BOOL reuseAddr = true;
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*)& reuseAddr, sizeof(reuseAddr)) == SOCKET_ERROR) {
		//LOG(fatal) << "Failed to set server socket option (SO_REUSEADDR) (" << WSAGetLastError() << ").";
		return 2;
	}

	struct sockaddr_in addr = { 0 };
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	inet_pton(AF_INET, host, &addr.sin_addr);
	//addr.sin_addr.s_addr = INADDR_ANY;

	if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
		//LOG(fatal) << "Failed to bind TCP server socket to (" << host << ":" << port << ") (" << WSAGetLastError() << ").";
		return 3;
	}

	if (listen(sock, 10) == SOCKET_ERROR) {
		//LOG(fatal) << "Failed to listen on TCP server socket (" << WSAGetLastError() << ").";
		return 4;
	}

	return 0;
}

void RPGNet::Server::TCPServer::Cleanup() {
	closesocket(sock);
	sock = 0;
}

bool RPGNet::Server::TCPServer::IsAlive() {
	return sock > 0;
}

RPGNet::Selectable* RPGNet::Server::TCPServer::AcceptConnection(Server* server) {
	struct sockaddr_in addr;
	int addrlen = sizeof(addr);
	SOCKET newSocket = accept(sock, (struct sockaddr*)&addr, &addrlen);
	if (newSocket == INVALID_SOCKET) {
		//LOG(fatal) << "Failed to accept socket on TCP connection (" << WSAGetLastError() << ").";
		return nullptr;
	}

	return server->m_createClient(server, newSocket, addr);
}

int RPGNet::Server::UDPServer::Initialize() {
	if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET) {
		//LOG(fatal) << "Failed to create TCP server socket (" << WSAGetLastError() << ").";
		return 1;
	}

	BOOL reuseAddr = true;
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*)&reuseAddr, sizeof(reuseAddr)) == SOCKET_ERROR) {
		//LOG(fatal) << "Failed to set server socket option (SO_REUSEADDR) (" << WSAGetLastError() << ").";
		return 2;
	}

	struct sockaddr_in addr = { 0 };
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = INADDR_ANY;

	if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
		//LOG(fatal) << "Failed to bind TCP server socket to (" << host << ":" << port << ") (" << WSAGetLastError() << ").";
		return 3;
	}

	return 0;
}

void RPGNet::Server::UDPServer::Cleanup() {
	closesocket(sock);
	sock = 0;
}

bool RPGNet::Server::UDPServer::IsAlive() {
	return sock > 0;
}

void RPGNet::Server::UDPServer::ReadPacket() {

}
