#pragma once

#include "WasabiGame/Networking/Selectable.hpp"
#include "WasabiGame/Networking/NetworkListener.hpp"
#include "WasabiGame/Utilities/Semaphore.hpp"
#include "WasabiGame/Utilities/CircularBuffer.hpp"

#if (defined WIN32 || defined _WIN32)
#define NOMINMAX
#include <WinSock2.h>
#include <WS2tcpip.h>
#endif

#include <string>
#include <atomic>


namespace WasabiGame {

	class NetworkListener;

	class NetworkClient : public Selectable {
	protected:
		std::shared_ptr<NetworkListener> m_listener;
		std::string m_hostname;
		int m_port;
		struct sockaddr_in m_destinationAddr;
		WasabiGame::Semaphore m_writingSemaphore;
		bool m_shouldDeregister;

		char m_writePacketBuffer[MAX_PACKET_SIZE]; // buffer to hold packet data before being sent
		char m_readPacketBuffer[MAX_PACKET_SIZE]; // buffer to hold packet data before being read
		WasabiGame::CircularBuffer m_outBuffer;
		std::function<bool(char*, size_t)> m_consumeBufferCallback;

		size_t WriteFragmentToBuffer(char* data, size_t len, size_t dataOffset, size_t totalWrittenToBuffer) {
			if (totalWrittenToBuffer >= dataOffset + len || totalWrittenToBuffer < dataOffset)
				return 0; // we already wrote past this point, or we didn't write enough to be at the dataOffset for this

			size_t remainingLenToWrite = dataOffset + len - totalWrittenToBuffer;
			size_t writeSize = std::min(m_outBuffer.GetAvailableContigiousInsert(), remainingLenToWrite);

			memcpy(m_outBuffer.GetWritingMem(), data + (len - remainingLenToWrite), writeSize);
			m_outBuffer.OnInserted(writeSize);
			return writeSize;
		}

		size_t ReadFragmentFromBuffer(char* dstBuffer, size_t len) {
			size_t totalRead = 0;
			while (totalRead < len) {
				size_t readSize = std::min(m_outBuffer.GetAvailableContigiousConsume(), len - totalRead);
				memcpy(dstBuffer + totalRead, m_outBuffer.GetReadingMem(), readSize);
				totalRead += readSize;
				m_outBuffer.OnConsumed(readSize);
			}
			return totalRead;
		}

	public:
		NetworkClient(std::shared_ptr<NetworkListener> listener) : Selectable(0), m_consumeBufferCallback(nullptr) {
			m_shouldDeregister = false;
			m_listener = listener;
			m_hostname = "";
			m_port = -1;

			m_outBuffer.Initialize(m_listener->Config->Get<size_t>("clientBufferSize"));
		}

		NetworkClient(std::shared_ptr<NetworkListener> listener, SOCKET sock, struct sockaddr_in addr) : Selectable(sock), m_consumeBufferCallback(nullptr) {
			m_shouldDeregister = false;
			m_listener = listener;
			char hostname[256];
			inet_ntop(AF_INET, &addr.sin_addr, hostname, sizeof(hostname));
			m_hostname = hostname;
			m_port = ntohs(addr.sin_port);

			m_outBuffer.Initialize(m_listener->Config->Get<size_t>("clientBufferSize"));
		}

		virtual ~NetworkClient() {
			Close();
		}

		virtual bool ShouldDeregisterSelectable() {
			return m_fd == 0 && m_shouldDeregister;
		}

		virtual int Connect(std::string hostname, int port) {
			SOCKET sock;
			struct sockaddr_in destinationAddr;
			destinationAddr.sin_port = htons(port);
			destinationAddr.sin_family = AF_INET;
			inet_pton(AF_INET, hostname.c_str(), &destinationAddr.sin_addr);

			LOG_F(INFO, "Attempting to connect to TCP server at %s:%d...", hostname.c_str(), port);

			if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
				LOG_F(ERROR, "Failed to create client socket (%d).", WSAGetLastError());
				return 1;
			}

			if (connect(sock, (struct sockaddr*)&destinationAddr, sizeof(destinationAddr)) == SOCKET_ERROR) {
				closesocket(sock);
				return 2;
			}

			m_fd = sock;
			m_hostname = hostname;
			m_port = port;
			m_destinationAddr = destinationAddr;

			m_outBuffer.Clear();
			m_listener->RegisterSelectable(shared_from_this());

			return 0;
		}

		virtual int Connect(struct sockaddr_in destinationAddr) {
			char hostname[256];
			inet_ntop(AF_INET, &destinationAddr.sin_addr, hostname, sizeof(hostname));
			return Connect(hostname, ntohs(destinationAddr.sin_port));
		}

		virtual void Close() {
			if (m_fd > 0) {
				LOG_F(INFO, "Closing client socket (fd=%d)", m_fd);
				closesocket(m_fd);
				m_fd = 0;
			}
			m_shouldDeregister = true;
		}

		virtual bool HasPendingWrites() {
			return m_outBuffer.GetSize() > 0;
		}

		/*virtual bool OnReadReady() {
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

		virtual void Write(const char* data, size_t len, sockaddr* addrTo = nullptr, int toAddrLen = 0) {
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

			m_listener->NotifyWriteAvailable();
		}*/

		virtual void Write(const char* data, size_t len, sockaddr* addrTo = nullptr, int toAddrLen = 0) {
			// will write toAddrLen (1 byte), then dataLen (2 bytes), then addrTo (length of toAddrLen) and then data (dataLen)
			m_writingSemaphore.wait();

			size_t totalToWrite = len + 2;
			size_t totalWritten = 0;
			while (totalWritten < totalToWrite) {
				int writeSize = std::min(m_outBuffer.GetAvailableContigiousInsert(), totalToWrite - totalWritten);
				if (writeSize == 0) {
					m_outBuffer.Expand();
					continue;
				}

				uint16_t dataLenShort = (uint16_t)len;
				totalWritten += WriteFragmentToBuffer((char*)&dataLenShort, 2, 0, totalWritten);
				totalWritten += WriteFragmentToBuffer((char*)data, len, 2, totalWritten);
			}

			m_writingSemaphore.notify();

			m_listener->NotifyWriteAvailable();
		}

		virtual bool OnWriteReady() {
			if (m_outBuffer.GetSize() < 2) {
				return true; // nothing to be written (need at least 2 bytes for sizes to be available)
			}

			uint8_t lengthBytes[2] = { m_outBuffer.PeakByteAt(0), m_outBuffer.PeakByteAt(1) };
			uint16_t dataLen;
			memcpy(&dataLen, &lengthBytes, 2);

			if (m_outBuffer.GetSize() >= 2 + dataLen) {
				// we have an entire packet ready in the buffer, so we can write it
				m_outBuffer.OnConsumed(2); // consume the 2 bytes we peaked earlier
				ReadFragmentFromBuffer(m_writePacketBuffer, dataLen);

				int numWritten;

				while (true) {
					numWritten = send(m_fd, m_writePacketBuffer, dataLen, 0);
					if (numWritten < 0) {
						// handle socket error
						int err = WSAGetLastError();
						if (err == WSAEINTR || err == WSAEINPROGRESS) {
							continue; // retry those errors
						}
					}
					break;
				}

				if (numWritten == 0 && m_fd > 0)
					Close();

				return numWritten > 0;
			} else
				return true; // buffer doesn't contain a full packet yet
		}

		virtual bool OnReadReady() {
			size_t numRead;

			while (true) {
				numRead = recv(m_fd, m_readPacketBuffer, MAX_PACKET_SIZE, 0);
				if (numRead < 0) {
					// handle socket error
					int err = WSAGetLastError();
					if (err == WSAEINTR || err == WSAEINPROGRESS) {
						continue; // retry those errors
					}
				}
				break;
			}

			if (numRead > 0 && m_consumeBufferCallback)
				return m_consumeBufferCallback(m_readPacketBuffer, numRead);

			if (numRead == 0 && m_fd > 0)
				Close();

			return numRead > 0;
		}

		void SetConsumeBufferCallback(std::function<bool(char*, size_t)> callback) {
			m_consumeBufferCallback = callback;
		}
	};

	class ReconnectingNetworkClient : public NetworkClient {
		WasabiGame::Semaphore m_connectingSemaphore;
		std::atomic<bool> m_reconnect;
		std::function<void()> m_onConnecting;
		std::function<void()> m_onConnectionFailed;
		std::function<void()> m_onConnected;
		std::function<void()> m_onDisconnected;

	public:
		ReconnectingNetworkClient(std::shared_ptr<NetworkListener> listener) : NetworkClient(listener) {
			m_reconnect.store(false);
			m_onConnecting = nullptr;
			m_onConnectionFailed = nullptr;
			m_onConnected = nullptr;
			m_onDisconnected = nullptr;
		}

		virtual int Connect(std::string hostname, int port) override {
			m_connectingSemaphore.wait();
			if (m_onConnecting)
				m_onConnecting();
			int ret = 0;
			if (m_hostname != hostname || m_port != port)
				Close();
			m_reconnect.store(true);
			if (fd() == 0)
				ret = NetworkClient::Connect(hostname, port);
			if (ret == 0 && m_onConnected)
				m_onConnected();
			else if (ret != 0 && m_onConnectionFailed)
				m_onConnectionFailed();
			m_connectingSemaphore.notify();
			return ret;
		}

		virtual int Connect(struct sockaddr_in destinationAddr) override {
			char hostname[256];
			inet_ntop(AF_INET, &destinationAddr.sin_addr, hostname, sizeof(hostname));
			return Connect(hostname, ntohs(destinationAddr.sin_port));
		}

		void Reconnect() {
			m_listener->Scheduler->LaunchThread("reconnect-client-" + std::to_string((uintptr_t)this), [this]() {
				while (this->fd() == 0 && m_listener->IsRunning() && this->m_reconnect.load()) {
					this->Connect(m_destinationAddr);
					if (this->fd() == 0 && m_listener->IsRunning())
						std::this_thread::sleep_for(std::chrono::milliseconds(1000));
				}
			});
		}

		void StopReconnecting() {
			m_reconnect.store(false);
			m_shouldDeregister = true;
		}

		virtual void Close() override {
			NetworkClient::Close();
			m_shouldDeregister = !m_reconnect.load();
		}

		virtual bool OnReadReady() override {
			bool bKeep = NetworkClient::OnReadReady();
			if (!bKeep && m_listener->IsRunning()) {
				if (m_onDisconnected)
					m_onDisconnected();
				Reconnect();
			}
			return bKeep;
		}

		virtual bool OnWriteReady() override {
			bool bKeep = NetworkClient::OnWriteReady();
			if (!bKeep && m_listener->IsRunning()) {
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

	class UDPNetworkClient : public NetworkClient {
	public:
		UDPNetworkClient(std::shared_ptr<NetworkListener> listener) : NetworkClient(listener) {
		}

		virtual int Connect(std::string hostname, int port) override {
			SOCKET sock;

			LOG_F(INFO, "Attempting to connect to UDP endpoint at %s:%d...", hostname.c_str(), port);

			if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET) {
				LOG_F(ERROR, "Failed to create TCP server socket (%d).", WSAGetLastError());
				return 1;
			}

			struct sockaddr_in destinationAddr;
			destinationAddr.sin_port = htons(port);
			destinationAddr.sin_family = AF_INET;
			inet_pton(AF_INET, hostname.c_str(), &destinationAddr.sin_addr);
			connect(sock, (struct sockaddr*)&destinationAddr, sizeof(destinationAddr));

			m_fd = sock;
			m_hostname = hostname;
			m_port = ntohs(port);
			m_destinationAddr = destinationAddr;

			m_outBuffer.Clear();
			m_listener->RegisterSelectable(shared_from_this());

			return 0;
		}

		virtual int Connect(struct sockaddr_in destinationAddr) override {
			char hostname[256];
			inet_ntop(AF_INET, &destinationAddr.sin_addr, hostname, sizeof(hostname));
			return Connect(hostname, ntohs(destinationAddr.sin_port));
		}
	};

	class UDPServerSideNetworkClient : public NetworkClient {

		sockaddr_storage m_addrBuffer; // buffer to hold serialized/deserialized addr
		std::function<void(void*, size_t, sockaddr*, int)> m_consumeUDPPacketCallback;

	public:
		UDPServerSideNetworkClient(std::shared_ptr<NetworkListener> listener) : NetworkClient(listener), m_consumeUDPPacketCallback(nullptr) {
			listener->m_UDPServer.selectable = this;
		}

		void SetConsumeUDPPacketCallback(std::function<void(void*, size_t, sockaddr*, int)> callback) {
			m_consumeUDPPacketCallback = callback;
		}

		virtual int Connect(std::string hostname, int port) override {
			// this client cant connect (it uses socket made and owned by m_listener->m_UDPServer)
			return -1;
		}

		virtual int Connect(struct sockaddr_in destinationAddr) override {
			// this client cant connect (it uses socket made and owned by m_listener->m_UDPServer)
			return -1;
		}

		virtual void Close() override {
			// this client cant close socket (it uses socket made and owned by m_listener->m_UDPServer)
		}

		virtual void Write(const char* data, size_t len, sockaddr* addrTo, int addrToLen) override {
			// will write toAddrLen (1 byte), then dataLen (2 bytes), then addrTo (length of toAddrLen) and then data (dataLen)
			m_writingSemaphore.wait();

			size_t totalToWrite = len + addrToLen + 3;
			size_t totalWritten = 0;
			while (totalWritten < totalToWrite) {
				int writeSize = std::min(m_outBuffer.GetAvailableContigiousInsert(), totalToWrite - totalWritten);
				if (writeSize == 0) {
					m_outBuffer.Expand();
					continue;
				}

				uint8_t toAddrLenByte = (uint8_t)addrToLen;
				uint16_t dataLenShort = (uint16_t)len;
				totalWritten += WriteFragmentToBuffer((char*)&toAddrLenByte, 1, 0, totalWritten);
				totalWritten += WriteFragmentToBuffer((char*)&dataLenShort, 2, 1, totalWritten);
				totalWritten += WriteFragmentToBuffer((char*)addrTo, addrToLen, 3, totalWritten);
				totalWritten += WriteFragmentToBuffer((char*)data, len, 3 + addrToLen, totalWritten);
			}

			m_writingSemaphore.notify();

			m_listener->NotifyWriteAvailable();
		}

		virtual bool OnWriteReady() override {
			if (m_outBuffer.GetSize() < 3) {
				return true; // nothing to be written (need at least 3 bytes for sizes to be available)
			}

			uint8_t lengthBytes[3] = { m_outBuffer.PeakByteAt(0), m_outBuffer.PeakByteAt(1), m_outBuffer.PeakByteAt(2) };
			uint8_t addrLen = lengthBytes[0];
			uint16_t dataLen;
			memcpy(&dataLen, &lengthBytes[1], 2);

			if (m_outBuffer.GetSize() >= 3 + addrLen + dataLen) {
				// we have an entire packet ready in the buffer, so we can write it
				m_outBuffer.OnConsumed(3); // consume the 3 bytes we peaked earlier
				ReadFragmentFromBuffer((char*)&m_addrBuffer, addrLen);
				ReadFragmentFromBuffer(m_writePacketBuffer, dataLen);

				int numWritten;
				while (true) {
					numWritten = sendto(m_listener->m_UDPServer.sock, m_writePacketBuffer, dataLen, 0, (sockaddr*)&m_addrBuffer, addrLen);
					if (numWritten < 0) {
						// handle socket error
						int err = WSAGetLastError();
						if (err == WSAEINTR || err == WSAEINPROGRESS) {
							continue; // retry those errors
						}
					}
					break;
				}

				return numWritten > 0;
			} else
				return true; // buffer doesn't contain a full packet yet
		}

		virtual bool OnReadReady() override {
			sockaddr_storage addrFrom;
			int addrSize = sizeof(addrFrom);
			size_t numRead;
			
			while (true) {
				numRead = recvfrom(m_listener->m_UDPServer.sock, m_readPacketBuffer, MAX_PACKET_SIZE, 0, (sockaddr*)&addrFrom, &addrSize);
				if (numRead < 0) {
					// handle socket error
					int err = WSAGetLastError();
					if (err == WSAEINTR || err == WSAEINPROGRESS) {
						continue; // retry those errors
					}
				}
				break;
			}

			if (numRead > 0 && m_consumeUDPPacketCallback)
				m_consumeUDPPacketCallback(m_readPacketBuffer, numRead, (sockaddr*)&addrFrom, addrSize);

			return numRead > 0;
		}
	};
}
