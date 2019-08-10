#pragma once

#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

#include "RTBServer/Lib/Utilities/Scheduler.hpp"
#include "RTBServer/Lib/Utilities/Config.hpp"
#include "RTBServer/Lib/Utilities/Semaphore.hpp"
#include "RTBServer/Lib/Selectable.hpp"

#include <unordered_map>
#include <functional>

namespace RPGNet {

	typedef std::function<Selectable* (class Server*, SOCKET, struct sockaddr_in)> CreateClientFunction;

	class Server {
		bool m_isRunning;
		CreateClientFunction m_createClient;

		struct ClientMetadata {
			bool deleteOnDisconnect;
		};
		HBUtils::Semaphore m_registerSemaphore;
		std::vector<std::pair<Selectable*, ClientMetadata>> m_pendingNewlyRegisteredClients;
		std::unordered_map<Selectable*, ClientMetadata> m_clients;

		struct TCPServer {
			int port;
			char host[256];
			SOCKET sock;

			int Initialize() {
				if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
					//LOG(fatal) << "Failed to create TCP server socket (" << WSAGetLastError() << ").";
					return 1;
				}

				BOOL reuseAddr = true;
				if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*)& reuseAddr, sizeof(reuseAddr)) == SOCKET_ERROR) {
					//LOG(fatal) << "Failed to set server socket option (SO_REUSEADDR) (" << WSAGetLastError() << ").";
					closesocket(sock);
					sock = 0;
					return 2;
				}

				struct sockaddr_in addr = { 0 };
				addr.sin_family = AF_INET;
				addr.sin_port = htons(port);
				inet_pton(AF_INET, host, &addr.sin_addr);
				//addr.sin_addr.s_addr = INADDR_ANY;

				if (bind(sock, (struct sockaddr*) & addr, sizeof(addr)) == SOCKET_ERROR) {
					//LOG(fatal) << "Failed to bind TCP server socket to (" << host << ":" << port << ") (" << WSAGetLastError() << ").";
					closesocket(sock);
					sock = 0;
					return 3;
				}

				if (listen(sock, 10) == SOCKET_ERROR) {
					//LOG(fatal) << "Failed to listen on TCP server socket (" << WSAGetLastError() << ").";
					closesocket(sock);
					sock = 0;
					return 4;
				}

				return 0;
			}

			void Cleanup() {
				closesocket(sock);
				sock = 0;
			}

			bool IsAlive() {
				return sock > 0;
			}

			Selectable* AcceptConnection(Server* server) {
				struct sockaddr_in addr;
				int addrlen = sizeof(addr);
				SOCKET newSocket = accept(sock, (struct sockaddr*) & addr, &addrlen);
				if (newSocket == INVALID_SOCKET) {
					//LOG(fatal) << "Failed to accept socket on TCP connection (" << WSAGetLastError() << ").";
					return nullptr;
				}

				return server->m_createClient(server, newSocket, addr);
			}
		} m_TCPServer;

		struct UDPServer {
			int port;
			char host[256];
			SOCKET sock;

			int Initialize() {
				if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET) {
					//LOG(fatal) << "Failed to create TCP server socket (" << WSAGetLastError() << ").";
					return 1;
				}

				BOOL reuseAddr = true;
				if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*)& reuseAddr, sizeof(reuseAddr)) == SOCKET_ERROR) {
					//LOG(fatal) << "Failed to set server socket option (SO_REUSEADDR) (" << WSAGetLastError() << ").";
					closesocket(sock);
					sock = 0;
					return 2;
				}

				struct sockaddr_in addr = { 0 };
				addr.sin_family = AF_INET;
				addr.sin_port = htons(port);
				addr.sin_addr.s_addr = INADDR_ANY;

				if (bind(sock, (struct sockaddr*) & addr, sizeof(addr)) == SOCKET_ERROR) {
					//LOG(fatal) << "Failed to bind TCP server socket to (" << host << ":" << port << ") (" << WSAGetLastError() << ").";
					closesocket(sock);
					sock = 0;
					return 3;
				}

				return 0;
			}

			void Cleanup() {
				closesocket(sock);
				sock = 0;
			}

			bool IsAlive() {
				return sock > 0;
			}

			void ReadPacket() {

			}
		} m_UDPServer;

	public:
		Server(CreateClientFunction createClient) : Config(), Scheduler() {
			m_isRunning = true;
			m_createClient = createClient;
			m_TCPServer = { 0 };
			m_UDPServer = { 0 };
		}

		~Server() {
		}

		HBUtils::Scheduler Scheduler;
		HBUtils::Config Config;

		void RegisterSelectable(Selectable* client, bool deleteOnDisconnect = true) {
			ClientMetadata meta;
			meta.deleteOnDisconnect = deleteOnDisconnect;
			m_registerSemaphore.wait();
			m_pendingNewlyRegisteredClients.push_back(std::make_pair(client, meta));
			m_registerSemaphore.notify();
			NotifyWriteAvailable();
		}

		void Run() {
			WSADATA wsa;
			if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
				// LOG(fatal) << "Failed to initialize WinSock.";
				return;
			}

			m_TCPServer.port = Config.Get<int>("tcpPort");
			m_UDPServer.port = Config.Get<int>("udpPort");

			strcpy_s(m_TCPServer.host, sizeof(m_TCPServer.host), Config.Get<char*>("hostname"));
			strcpy_s(m_UDPServer.host, sizeof(m_UDPServer.host), Config.Get<char*>("hostname"));

			if (m_TCPServer.port <= 0 || m_TCPServer.Initialize() == 0) {
				if (m_UDPServer.port <= 0 || m_UDPServer.Initialize() == 0) {
					Scheduler.LaunchWorkers(Config.Get<int>("numWorkers"));
					Scheduler.LaunchThread("selectables-loop", [this]() {
						this->SelectablesLoop();
						this->Scheduler.Stop();
					});
					Scheduler.Run();

					if (m_UDPServer.port > 0)
						m_UDPServer.Cleanup();
				}
				if (m_TCPServer.port > 0)
					m_TCPServer.Cleanup();
			}

			WSACleanup();
		}

		void Stop() {
			m_isRunning = false;
		}

		// Called to notify the server that a write is now available (and select should be re-called)
		void NotifyWriteAvailable() {
			// @TODO: implement (write to an fd that select always wants to read from, and dump data away)
		}

	private:

		void SelectablesLoop() {
			std::vector<std::pair<Selectable*, ClientMetadata>> clientsToDelete;
			while (m_isRunning) {
				fd_set readFDs, writeFDs;
				int maxFDs = 0;

				FD_ZERO(&readFDs);
				FD_ZERO(&writeFDs);

				m_registerSemaphore.wait();
				if (m_pendingNewlyRegisteredClients.size() > 0) {
					for (auto client : m_pendingNewlyRegisteredClients)
						m_clients.insert(client);
					m_pendingNewlyRegisteredClients.clear();
				}
				m_registerSemaphore.notify();

				if (m_TCPServer.IsAlive()) {
					FD_SET(m_TCPServer.sock, &readFDs);
					maxFDs = max(maxFDs, (int)m_TCPServer.sock);
				}
				if (m_UDPServer.IsAlive()) {
					FD_SET(m_UDPServer.sock, &readFDs);
					maxFDs = max(maxFDs, (int)m_UDPServer.sock);
				}

				for (auto client : m_clients) {
					if (client.first->fd() > 0) {
						maxFDs = max(maxFDs, client.first->fd());
						FD_SET(client.first->fd(), &readFDs);
						if (client.first->HasPendingWrites())
							FD_SET(client.first->fd(), &writeFDs);
					}
				}

				if (maxFDs > 0) {
					select(maxFDs, &readFDs, &writeFDs, NULL, NULL);

					if (FD_ISSET(m_TCPServer.sock, &readFDs)) {
						// pending TCP connection
						Selectable* newClient = m_TCPServer.AcceptConnection(this);
						if (newClient)
							RegisterSelectable(newClient, true);
					}
					if (FD_ISSET(m_UDPServer.sock, &readFDs)) {
						// pending UDP packet
						m_UDPServer.ReadPacket();
					}

					for (auto client : m_clients) {
						if (FD_ISSET(client.first->fd(), &readFDs)) {
							if (!client.first->OnReadReady()) {
								clientsToDelete.push_back(client);
							}
						}
						if (FD_ISSET(client.first->fd(), &writeFDs)) {
							if (!client.first->OnWriteReady()) {
								clientsToDelete.push_back(client);
							}
						}
					}

					for (auto client : clientsToDelete) {
						m_clients.erase(client.first);
						if (client.second.deleteOnDisconnect)
							delete client.first;
					}
					clientsToDelete.clear();
				} else
					std::this_thread::sleep_for(std::chrono::milliseconds(200));
			}
		}
	};

	template<typename ClientType>
	class ServerT : public Server {
	public:
		ServerT() : Server([](Server* server, SOCKET sock, struct sockaddr_in addr) {
			return new ClientType(server, sock, addr);
		}) {}
	};

};
