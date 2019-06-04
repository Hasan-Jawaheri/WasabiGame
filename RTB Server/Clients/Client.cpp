#include "Client.hpp"
#include "../Server.hpp"
#include <WS2tcpip.h>

RPGNet::Client::Client(Server* server, SOCKET sock, struct sockaddr_in addr) : Selectable(sock) {
	m_server = server;
	char ip[256];
	inet_ntop(AF_INET, &addr.sin_addr, ip, sizeof(ip));
	m_IP = ip;
	m_port = ntohs(addr.sin_port);

	m_outBuffer.Initialize(m_server->Config.Get<size_t>("clientBufferSize"));
	m_inBuffer.Initialize(m_server->Config.Get<size_t>("clientBufferSize"));
}

RPGNet::Client::~Client() {
	if (m_fd > 0) {
		closesocket(m_fd);
		m_fd = 0;
	}
}

bool RPGNet::Client::HasPendingWrites() {
	return m_outBuffer.GetSize() > 0;
}

bool RPGNet::Client::OnReadReady() {
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

bool RPGNet::Client::OnWriteReady() {
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

	return numWritten > 0;
}

bool RPGNet::Client::ConsumeBuffer(CircularBuffer* buffer) {
	buffer->start = buffer->end; // instant consume, should be implemented elsewhere
	return true;
}

void RPGNet::Client::Write(char* data, size_t len) {
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

	m_server->NotifyWriteAvailable();
}

void RPGNet::CircularBuffer::Initialize(size_t _size) {
	mem = new char[_size];
	memSize = _size;
	start = 0;
	end = 0;
}

void RPGNet::CircularBuffer::Expand() {
	char* newBuf = new char[memSize * 2];
	memcpy(newBuf, mem + start, memSize - start);
	memcpy(newBuf + (memSize - start), mem, end);

	start = 0;
	end = memSize;
	memSize *= 2;
	delete[] mem;
	mem = newBuf;
}

void RPGNet::CircularBuffer::Destroy() {
	delete[] mem;
}

char* RPGNet::CircularBuffer::GetMem() {
	return mem + start;
}

size_t RPGNet::CircularBuffer::GetAvailableContigiousInsert() {
	// number of bytes available from "end" to "memSize", or "end" to "start" if "end" < "start"
	if (end >= start)
		return memSize - end;
	else
		return start - end;
}

size_t RPGNet::CircularBuffer::GetAvailableContigiousConsume() {
	// number of bytes from "start" to "end", or "start" to "memSize" if "end" < "start"
	if (end >= start)
		return end - start;
	else
		return memSize - start;
}

size_t RPGNet::CircularBuffer::GetSize() {
	if (end >= start)
		return end - start;
	else
		return (memSize - start) + end;
}

void RPGNet::CircularBuffer::OnInserted(size_t numInserted) {
	end = (end + numInserted) % memSize;
}

void RPGNet::CircularBuffer::OnConsumed(size_t numConsumed) {
	start = (start + numConsumed) % memSize;
}
