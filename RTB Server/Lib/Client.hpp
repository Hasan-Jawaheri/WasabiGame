#pragma once

#include "Selectable.hpp"
#include "Server.hpp"
#include "Utilities/Semaphore.hpp"
#include "Utilities/CircularBuffer.hpp"

#include <WinSock2.h>
#include <WS2tcpip.h>

#include <string>

namespace RPGNet {

	class Client : public Selectable {
		class Server* m_server;
		std::string m_IP;
		int m_port;
		HBUtils::Semaphore m_writingSemaphore;

		HBUtils::CircularBuffer m_outBuffer;
		HBUtils::CircularBuffer m_inBuffer;

	public:
		Client(class Server* server) : Selectable(0) {
			m_server = server;
			m_IP = "";
			m_port = -1;

			m_outBuffer.Initialize(m_server->Config.Get<size_t>("clientBufferSize"));
			m_inBuffer.Initialize(m_server->Config.Get<size_t>("clientBufferSize"));
		}

		Client(class Server* server, SOCKET sock, struct sockaddr_in addr) : Selectable(sock) {
			m_server = server;
			char ip[256];
			inet_ntop(AF_INET, &addr.sin_addr, ip, sizeof(ip));
			m_IP = ip;
			m_port = ntohs(addr.sin_port);

			m_outBuffer.Initialize(m_server->Config.Get<size_t>("clientBufferSize"));
			m_inBuffer.Initialize(m_server->Config.Get<size_t>("clientBufferSize"));
		}

		~Client() {
			if (m_fd > 0) {
				closesocket(m_fd);
				m_fd = 0;
			}
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

			m_server->RegisterSelectable(this, false);

			return 0;
		}

		virtual bool HasPendingWrites() {
			return m_outBuffer.GetSize() > 0;
		}

		virtual bool OnReadReady() {
			int numRead = 1, totalRead = 0;
			while (numRead > 0) {
				int readSize = m_inBuffer.GetAvailableContigiousInsert();
				if (readSize == 0) {
					m_inBuffer.Expand();
					continue;
				}

				numRead = recv(m_fd, m_inBuffer.GetMem(), readSize, 0);
				if (numRead > 0) {
					m_inBuffer.OnInserted(numRead);
					totalRead += numRead;
				} else if (numRead < 0) {
					// handle socket error
					int err = WSAGetLastError();
					if (err == WSAEINTR || err == WSAEINPROGRESS) {
						numRead = 1; // will effectively retry
					}
				}
			}

			if (totalRead > 0)
				return ConsumeBuffer(&m_inBuffer);

			return numRead > 0;
		}

		virtual bool OnWriteReady() {
			m_writingSemaphore.wait();

			int numWritten = 1, totalWritten = 0;
			while (m_outBuffer.GetSize() > 0 && numWritten > 0) {
				int writeSize = m_outBuffer.GetAvailableContigiousConsume();
				numWritten = send(m_fd, m_outBuffer.GetMem(), writeSize, 0);
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

			m_writingSemaphore.notify();

			return numWritten > 0;
		}

		virtual bool ConsumeBuffer(HBUtils::CircularBuffer* buffer) {
			buffer->start = buffer->end; // instant consume, should be implemented elsewhere
			return true;
		}

		void Write(const char* data, size_t len) {
			m_writingSemaphore.wait();

			size_t totalWritten = 0;
			while (totalWritten < len) {
				int writeSize = min(m_outBuffer.GetAvailableContigiousInsert(), len - totalWritten);
				if (writeSize == 0) {
					m_outBuffer.Expand();
					continue;
				}

				memcpy(m_outBuffer.GetMem(), data + totalWritten, writeSize);
				m_outBuffer.OnInserted(writeSize);
				totalWritten += writeSize;
			}

			m_writingSemaphore.notify();

			m_server->NotifyWriteAvailable();
		}
	};

};
