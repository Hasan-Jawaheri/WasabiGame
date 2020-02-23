#pragma once

#if (defined WIN32 || defined _WIN32)
#define NOMINMAX
#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#endif

#include "WasabiGame/Utilities/Scheduler.hpp"
#include "WasabiGame/Utilities/Config.hpp"
#include "WasabiGame/Utilities/Semaphore.hpp"
#include "WasabiGame/Networking/Selectable.hpp"

#include <unordered_map>
#include <functional>


namespace WasabiGame {

	class NetworkListener;

	typedef std::function<std::shared_ptr<Selectable> (std::shared_ptr<NetworkListener>, SOCKET, struct sockaddr_in)> CreateNetworkClientFunction;
	typedef std::function<void(std::shared_ptr<Selectable>)> NetworkClientConnectedFunction;
	typedef std::function<void(std::shared_ptr<Selectable>)> NetworkClientDisconnectedFunction;

	class NetworkListener : public std::enable_shared_from_this<NetworkListener> {
		bool m_isRunning;
		CreateNetworkClientFunction m_createClient;
		NetworkClientConnectedFunction m_clientConnected;
		NetworkClientDisconnectedFunction m_clientDisconnected;

		struct ClientMetadata {
			bool deleteOnDisconnect;
		};
		WasabiGame::Semaphore m_registerSemaphore;
		std::vector<std::pair<std::shared_ptr<Selectable>, ClientMetadata>> m_pendingNewlyRegisteredClients;
		std::unordered_map<std::shared_ptr<Selectable>, ClientMetadata> m_clients;

		struct TCPServer {
			int port;
			char host[256];
			SOCKET sock;

			int Initialize() {
				struct sockaddr_in addr = { 0 };
				addr.sin_family = AF_INET;
				addr.sin_port = htons(port);
				inet_pton(addr.sin_family, host, &addr.sin_addr);
				//addr.sin_addr.s_addr = htonl(INADDR_ANY);

				if ((sock = socket(addr.sin_family, SOCK_STREAM, 0)) == INVALID_SOCKET) {
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

				if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
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

			std::shared_ptr<Selectable> AcceptConnection(std::shared_ptr<NetworkListener> server) {
				struct sockaddr_in addr;
				int addrlen = sizeof(addr);
				SOCKET newSocket = accept(sock, (struct sockaddr*)&addr, &addrlen);
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
				struct sockaddr_in addr = { 0 };
				addr.sin_family = AF_INET;
				addr.sin_port = htons(port);
				addr.sin_addr.s_addr = htonl(INADDR_ANY);

				if ((sock = socket(addr.sin_family, SOCK_DGRAM, 0)) == INVALID_SOCKET) {
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
		NetworkListener(std::shared_ptr<GameConfig> config, std::shared_ptr<GameScheduler> scheduler, CreateNetworkClientFunction createClient) : enable_shared_from_this<NetworkListener>(), Config(config), Scheduler(scheduler) {
			m_isRunning = true;
			m_createClient = createClient;
			m_clientConnected = nullptr;
			m_clientDisconnected = nullptr;
			m_TCPServer = { 0 };
			m_UDPServer = { 0 };
		}

		virtual ~NetworkListener() {
		}

		std::shared_ptr<GameConfig> Config;
		std::shared_ptr<GameScheduler> Scheduler;

		void RegisterSelectable(std::shared_ptr<Selectable> client, bool deleteOnDisconnect = true) {
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

			m_TCPServer.port = Config->Get<int>("tcpPort");
			m_UDPServer.port = Config->Get<int>("udpPort");

			strcpy_s(m_TCPServer.host, sizeof(m_TCPServer.host), Config->Get<char*>("hostname"));
			strcpy_s(m_UDPServer.host, sizeof(m_UDPServer.host), Config->Get<char*>("hostname"));

			if (m_TCPServer.port <= 0 || m_TCPServer.Initialize() == 0) {
				if (m_UDPServer.port <= 0 || m_UDPServer.Initialize() == 0) {
					Scheduler->LaunchWorkers(Config->Get<int>("numWorkers"));
					Scheduler->LaunchThread("selectables-loop", [this]() {
						this->SelectablesLoop();
						this->Scheduler->Stop();
					});
					Scheduler->Run();

					for (auto client : m_clients) {
						if (client.second.deleteOnDisconnect) {
							if (m_clientDisconnected)
								m_clientDisconnected(client.first);
						}
					}
					m_clients.clear();

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
			NotifyWriteAvailable();
		}

		bool IsRunning() const {
			return m_isRunning;
		}

		// Called to notify the server that a write is now available (and select should be re-called)
		void NotifyWriteAvailable() {
			// @TODO: implement (write to an fd that select always wants to read from, and dump data away)
		}

		void SetOnClientConnected(NetworkClientConnectedFunction callback) {
			m_clientConnected = callback;
		}

		void SetOnClientDisconnected(NetworkClientDisconnectedFunction callback) {
			m_clientDisconnected = callback;
		}

	private:

		void SelectablesLoop() {
			std::vector<std::pair<std::shared_ptr<Selectable>, ClientMetadata>> clientsToDelete;
			while (m_isRunning && Scheduler->IsRunning()) {
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
					maxFDs = std::max(maxFDs, (int)m_TCPServer.sock);
				}
				if (m_UDPServer.IsAlive()) {
					FD_SET(m_UDPServer.sock, &readFDs);
					maxFDs = std::max(maxFDs, (int)m_UDPServer.sock);
				}

				for (auto client : m_clients) {
					if (client.first->fd() > 0) {
						maxFDs = std::max(maxFDs, client.first->fd());
						FD_SET(client.first->fd(), &readFDs);
						if (client.first->HasPendingWrites())
							FD_SET(client.first->fd(), &writeFDs);
					}
				}

				if (maxFDs > 0) {
					timeval timeout = { 1, 0 };
					int numDescriptors = select(maxFDs, &readFDs, &writeFDs, NULL, &timeout);

					if (numDescriptors > 0) {
						if (FD_ISSET(m_TCPServer.sock, &readFDs)) {
							// pending TCP connection
							std::shared_ptr<Selectable> newClient = m_TCPServer.AcceptConnection(shared_from_this());
							if (newClient) {
								RegisterSelectable(newClient, true);
								if (m_clientConnected)
									m_clientConnected(newClient);
							}
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
							if (client.second.deleteOnDisconnect) {
								if (m_clientDisconnected)
									m_clientDisconnected(client.first);
							}
						}
						clientsToDelete.clear();
					}
				} else
					std::this_thread::sleep_for(std::chrono::milliseconds(200));
			}
		}
	};

	template<typename ClientType>
	class NetworkListenerT : public NetworkListener {
		static std::shared_ptr<Selectable> createClient(std::shared_ptr<NetworkListener> listener, SOCKET sock, struct sockaddr_in addr) {
			return std::make_shared<ClientType>(listener, sock, addr);
		}

	public:
		NetworkListenerT(std::shared_ptr<GameConfig> config, std::shared_ptr<GameScheduler> scheduler) : NetworkListener(
			config,
			scheduler,
			(CreateNetworkClientFunction)(NetworkListenerT<ClientType>::createClient)) {
		}
	};

};
