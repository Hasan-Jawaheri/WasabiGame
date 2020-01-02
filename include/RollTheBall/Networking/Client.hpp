#pragma once

#include "RollTheBall/Networking/Selectable.hpp"
#include "RollTheBall/Networking/Server.hpp"
#include "RollTheBall/Utilities/Semaphore.hpp"
#include "RollTheBall/Utilities/CircularBuffer.hpp"

#define NOMINMAX
#include <WinSock2.h>
#include <WS2tcpip.h>

#include <string>
#include <atomic>

namespace RPGNet {

	class Client : public Selectable {
	protected:
		class Server* m_server;
		std::string m_IP;
		int m_port;
		HBUtils::Semaphore m_writingSemaphore;

		HBUtils::CircularBuffer m_outBuffer;
		HBUtils::CircularBuffer m_inBuffer;
		std::function<bool(HBUtils::CircularBuffer*)> m_consumeBufferCallback;

	public:
		Client(class Server* server) : Selectable(0) {
			m_server = server;
			m_IP = "";
			m_port = -1;

			m_outBuffer.Initialize(m_server->Config.Get<size_t>("clientBufferSize"));
			m_inBuffer.Initialize(m_server->Config.Get<size_t>("clientBufferSize"));
		}

		Client(class Server* server, SOCKET sock, struct sockaddr_in addr) : Selectable(sock), m_consumeBufferCallback(nullptr) {
			m_server = server;
			char ip[256];
			inet_ntop(AF_INET, &addr.sin_addr, ip, sizeof(ip));
			m_IP = ip;
			m_port = ntohs(addr.sin_port);

			m_outBuffer.Initialize(m_server->Config.Get<size_t>("clientBufferSize"));
			m_inBuffer.Initialize(m_server->Config.Get<size_t>("clientBufferSize"));
		}

		virtual ~Client() {
			Close();
		}

		virtual int Connect(std::string hostname, int port) {
			SOCKET sock;

			if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
				//LOG(fatal) << "Failed to create client socket (" << WSAGetLastError() << ").";
				return 1;
			}

			struct sockaddr_in addr;
			addr.sin_port = htons(port);
			addr.sin_family = AF_INET;
			inet_pton(AF_INET, hostname.c_str(), &addr.sin_addr);
			if (connect(sock, (struct sockaddr*) & addr, sizeof(addr)) == SOCKET_ERROR) {
				closesocket(sock);
				return 2;
			}

			m_fd = sock;
			m_port = port;
			m_IP = hostname;

			m_inBuffer.Clear();
			m_outBuffer.Clear();
			m_server->RegisterSelectable(this, false);

			return 0;
		}

		virtual void Close() {
			if (m_fd > 0) {
				closesocket(m_fd);
				m_fd = 0;
			}
		}

		virtual bool HasPendingWrites() {
			return m_outBuffer.GetSize() > 0;
		}

		virtual bool OnReadReady() {
			int numRead = m_fd > 0 ? 1 : 0;
			while (numRead > 0) {
				int readSize = m_inBuffer.GetAvailableContigiousInsert();
				if (readSize == 0) {
					m_inBuffer.Expand();
					continue;
				}

				numRead = recv(m_fd, m_inBuffer.GetWritingMem(), readSize, 0);
				if (numRead > 0) {
					m_inBuffer.OnInserted(numRead);
					break;
				} else if (numRead < 0) {
					// handle socket error
					int err = WSAGetLastError();
					if (err == WSAEINTR || err == WSAEINPROGRESS) {
						numRead = 1; // will effectively retry
					}
				}
			}

			if (numRead > 0)
				return ConsumeBuffer(&m_inBuffer);

			if (numRead == 0 && m_fd > 0)
				Close();

			return numRead > 0;
		}

		virtual bool OnWriteReady() {
			int numWritten = m_fd > 0 ? 1 : 0, totalWritten = 0;
			while (m_outBuffer.GetSize() > 0 && numWritten > 0) {
				int writeSize = m_outBuffer.GetAvailableContigiousConsume();
				numWritten = send(m_fd, m_outBuffer.GetReadingMem(), writeSize, 0);
				if (numWritten > 0) {
					m_outBuffer.OnConsumed(numWritten);
					totalWritten += numWritten;
				} else if (numWritten < 0) {
					// handle socket error
					int err = WSAGetLastError();
					if (err == WSAEINTR || err == WSAEINPROGRESS) {
						numWritten = 1; // will effectively retry
					}
				}
			}

			if (numWritten == 0 && m_fd > 0)
				Close();

			return numWritten > 0;
		}

		virtual bool ConsumeBuffer(HBUtils::CircularBuffer* buffer) {
			if (m_consumeBufferCallback) {
				return m_consumeBufferCallback(buffer);
			}
			buffer->start = buffer->end; // instant consume, should be implemented elsewhere
			return true;
		}

		void SetConsumeBufferCallback(std::function<bool(HBUtils::CircularBuffer*)> callback) {
			m_consumeBufferCallback = callback;
		}

		void Write(const char* data, size_t len) {
			m_writingSemaphore.wait();

			size_t totalWritten = 0;
			while (totalWritten < len) {
				int writeSize = std::min(m_outBuffer.GetAvailableContigiousInsert(), len - totalWritten);
				if (writeSize == 0) {
					m_outBuffer.Expand();
					continue;
				}

				memcpy(m_outBuffer.GetWritingMem(), data + totalWritten, writeSize);
				m_outBuffer.OnInserted(writeSize);
				totalWritten += writeSize;
			}

			m_writingSemaphore.notify();

			m_server->NotifyWriteAvailable();
		}
	};

	class ReconnectingClient : public Client {
		HBUtils::Semaphore m_connectingSemaphore;
		std::atomic<bool> m_reconnect;
		std::function<void()> m_onConnecting;
		std::function<void()> m_onConnectionFailed;
		std::function<void()> m_onConnected;
		std::function<void()> m_onDisconnected;

	public:
		ReconnectingClient(class Server* server) : Client(server) {
			m_reconnect = false;
			m_onConnecting = nullptr;
			m_onConnectionFailed = nullptr;
			m_onConnected = nullptr;
			m_onDisconnected = nullptr;
		}

		virtual int Connect(std::string hostname, int port) {
			m_connectingSemaphore.wait();
			if (m_onConnecting)
				m_onConnecting();
			int ret = 0;
			if (hostname != m_IP || port != m_port)
				Close();
			m_reconnect = true;
			if (fd() == 0)
				ret = Client::Connect(hostname, port);
			if (ret == 0 && m_onConnected)
				m_onConnected();
			else if (ret != 0 && m_onConnectionFailed)
				m_onConnectionFailed();
			m_connectingSemaphore.notify();
			return ret;
		}

		void Reconnect() {
			m_server->Scheduler.LaunchThread("reconnect-client-" + std::to_string((uintptr_t)this), [this]() {
				while (this->fd() == 0 && m_server->IsRunning() && this->m_reconnect.load()) {
					this->Connect(m_IP, m_port);
					if (this->fd() == 0 && m_server->IsRunning())
						std::this_thread::sleep_for(std::chrono::milliseconds(1000));
				}
			});
		}

		void StopReconnecting() {
			m_reconnect = false;
		}

		virtual void Close() {
			Client::Close();
		}

		virtual bool OnReadReady() {
			bool bKeep = Client::OnReadReady();
			if (!bKeep && m_server->IsRunning()) {
				if (m_onDisconnected)
					m_onDisconnected();
				Reconnect();
			}
			return bKeep;
		}

		virtual bool OnWriteReady() {
			bool bKeep = Client::OnWriteReady();
			if (!bKeep && m_server->IsRunning()) {
				if (m_onDisconnected)
					m_onDisconnected();
				Reconnect();
			}
			return bKeep;
		}

		void SetOnConnectedCallback(std::function<void()> callback) {
			m_onConnected = callback;
		}

		void SetOnDisconnectedCallback(std::function<void()> callback) {
			m_onDisconnected = callback;
		}

		void SetOnConnectingCallback(std::function<void()> callback) {
			m_onConnecting = callback;
		}

		void SetOnConnectionFailedCallback(std::function<void()> callback) {
			m_onConnectionFailed = callback;
		}
	};

};
