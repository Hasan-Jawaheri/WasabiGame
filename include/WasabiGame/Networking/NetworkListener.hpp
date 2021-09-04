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
		friend class UDPServerSideNetworkClient;

		bool m_isRunning;
		CreateNetworkClientFunction m_createClient;
		NetworkClientConnectedFunction m_clientConnected;
		NetworkClientDisconnectedFunction m_clientDisconnected;

		struct ClientMetadata {
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
					LOG_F(ERROR, "Failed to create TCP server socket (%d).", WSAGetLastError());
					return 1;
				}

				BOOL reuseAddr = true;
				if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*)& reuseAddr, sizeof(reuseAddr)) == SOCKET_ERROR) {
					LOG_F(ERROR, "Failed to set server socket option (SO_REUSEADDR) (%d).", WSAGetLastError());
					closesocket(sock);
					sock = 0;
					return 2;
				}

				if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
					LOG_F(ERROR, "Failed to bind TCP server socket to (%s:%d) (%d).", host, port, WSAGetLastError());
					closesocket(sock);
					sock = 0;
					return 3;
				}

				if (listen(sock, 10) == SOCKET_ERROR) {
					LOG_F(ERROR, "Failed to listen on TCP server socket (%d).", WSAGetLastError());
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
					LOG_F(ERROR, "Failed to accept socket on TCP connection (%d).", WSAGetLastError());
					return nullptr;
				}

				return server->m_createClient(server, newSocket, addr);
			}
		} m_TCPServer;

		struct UDPServer {
			int port;
			char host[256];
			SOCKET sock;
			struct sockaddr_in addr;
			Selectable* selectable;

			int Initialize() {
				addr = { 0 };
				addr.sin_family = AF_INET;
				addr.sin_port = htons(port);
				addr.sin_addr.s_addr = htonl(INADDR_ANY);

				if ((sock = socket(addr.sin_family, SOCK_DGRAM, 0)) == INVALID_SOCKET) {
					LOG_F(ERROR, "Failed to create UDP server socket (%d).", WSAGetLastError());
					return 1;
				}

				BOOL reuseAddr = true;
				if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*)&reuseAddr, sizeof(reuseAddr)) == SOCKET_ERROR) {
					LOG_F(ERROR, "Failed to set server socket option (SO_REUSEADDR) (%d).", WSAGetLastError());
					closesocket(sock);
					sock = 0;
					return 2;
				}

				if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
					LOG_F(ERROR, "Failed to bind TCP server socket to (%s:%d) (%d).", host, port, WSAGetLastError());
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

		void RegisterSelectable(std::shared_ptr<Selectable> client) {
			ClientMetadata meta;
			m_registerSemaphore.wait();
			m_pendingNewlyRegisteredClients.push_back(std::make_pair(client, meta));
			m_registerSemaphore.notify();
			NotifyWriteAvailable();
		}

		void RunDetached() {
			WSADATA wsa;
			if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
				LOG_F(ERROR, "Failed to initialize WinSock.");
				return;
			}

			m_TCPServer.port = Config->Get<int>("tcpPort");
			m_UDPServer.port = Config->Get<int>("udpPort");

			strcpy_s(m_TCPServer.host, sizeof(m_TCPServer.host), Config->Get<char*>("hostname"));
			strcpy_s(m_UDPServer.host, sizeof(m_UDPServer.host), Config->Get<char*>("hostname"));

			if (m_TCPServer.port > 0 && m_TCPServer.Initialize() != 0)
				m_TCPServer.port = 0;
			if (m_UDPServer.port > 0 && m_UDPServer.Initialize() != 0)
				m_UDPServer.port = 0;

			Scheduler->LaunchWorkers(Config->Get<int>("numWorkers"));
			Scheduler->LaunchThread("selectables-loop", [this]() {
				this->SelectablesLoop();

				if (m_UDPServer.port > 0)
					m_UDPServer.Cleanup();
				if (m_TCPServer.port > 0)
					m_TCPServer.Cleanup();
			});
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
			/**
			 * TODO: Move HasPendingWrites() to before select (so don't FD_SET if no pending writes) but also add a read FD that is always part of select
			 * and when anything is written, we also write to that FD so it pings the select. This way the select is not a hot loop. In Linux we can use
			 * eventfd(), on Windows idk... maybe _pipe with 2 bytes size, so 1 can be written to and 1 can be read (at a time, after select), discard
			 * failed writes (if write fails, then 2 bytes are in pipe, so in select thread, we are either after-select, which means we will read one, so
			 * one is still in the pipe and will cause another select (good), or we are before-select, which means we will select twice, second time is
			 * possibly redundant, which is not bad)
			 */
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
				if (m_UDPServer.IsAlive() && m_UDPServer.selectable) {
					FD_SET(m_UDPServer.sock, &readFDs);
					FD_SET(m_UDPServer.sock, &writeFDs);
					maxFDs = std::max(maxFDs, (int)m_UDPServer.sock);
				}

				for (auto client : m_clients) {
					if (client.first->fd() > 0) {
						maxFDs = std::max(maxFDs, client.first->fd());
						FD_SET(client.first->fd(), &readFDs);
						FD_SET(client.first->fd(), &writeFDs);
					} else if (client.first->ShouldDeregisterSelectable())
						clientsToDelete.push_back(client);
				}

				if (maxFDs > 0) {
					timeval timeout = { 1, 0 };
					int numDescriptors = select(maxFDs, &readFDs, &writeFDs, NULL, &timeout);

					if (numDescriptors > 0) {
						if (FD_ISSET(m_TCPServer.sock, &readFDs)) {
							// pending TCP connection
							std::shared_ptr<Selectable> newClient = m_TCPServer.AcceptConnection(shared_from_this());
							if (newClient) {
								RegisterSelectable(newClient);
								if (m_clientConnected)
									m_clientConnected(newClient);
							}
						}

						if (m_UDPServer.selectable) {
							if (FD_ISSET(m_UDPServer.sock, &readFDs)) {
								if (!m_UDPServer.selectable->OnReadReady()) {
									LOG_F(FATAL, "UDP Server died while reading, unable to recover");
								}
							}

							if (FD_ISSET(m_UDPServer.sock, &writeFDs) && m_UDPServer.selectable->HasPendingWrites()) {
								if (!m_UDPServer.selectable->OnWriteReady()) {
									LOG_F(FATAL, "UDP Server died while writing, unable to recover");
								}
							}
						}

						for (auto client : m_clients) {
							if (FD_ISSET(client.first->fd(), &readFDs)) {
								if (!client.first->OnReadReady()) {
									clientsToDelete.push_back(client);
								}
							}
							if (FD_ISSET(client.first->fd(), &writeFDs) && client.first->HasPendingWrites()) {
								if (!client.first->OnWriteReady()) {
									clientsToDelete.push_back(client);
								}
							}
						}

						for (auto client : clientsToDelete) {
							m_clients.erase(client.first);
							if (m_clientDisconnected)
								m_clientDisconnected(client.first);
						}
						clientsToDelete.clear();
					}
				} else
					std::this_thread::sleep_for(std::chrono::milliseconds(200));
			}

			for (auto client : m_clients) {
				if (m_clientDisconnected)
					m_clientDisconnected(client.first);
			}
			m_clients.clear();
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
