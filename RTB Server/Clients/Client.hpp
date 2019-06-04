#pragma once

#include "../Selectable.hpp"
#include <WinSock2.h>

#include <string>

namespace RPGNet {

	struct CircularBuffer {
		char* mem;
		int start, end;
		size_t memSize;

		void Initialize(size_t _size);
		void Expand();
		void Destroy();

		inline char* GetMem();
		inline size_t GetAvailableContigiousInsert();
		inline size_t GetAvailableContigiousConsume();
		inline size_t GetSize();
		inline void OnInserted(size_t numInserted);
		inline void OnConsumed(size_t numConsumed);
	};

	class Client : public Selectable {
		class Server* m_server;
		std::string m_IP;
		int m_port;

		CircularBuffer m_outBuffer;
		CircularBuffer m_inBuffer;

	public:
		Client(class Server* server, SOCKET sock, struct sockaddr_in addr);
		~Client();

		virtual bool HasPendingWrites();
		virtual bool OnReadReady();
		virtual bool OnWriteReady();

		virtual bool ConsumeBuffer(CircularBuffer* buffer);
		void Write(char* data, size_t len);
	};

};
