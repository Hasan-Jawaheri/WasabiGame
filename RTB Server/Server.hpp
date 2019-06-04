#pragma once

#include <WinSock2.h>
#pragma comment(lib, "ws2_32.lib")

#include "Lib/Scheduler.hpp"
#include "Lib/Config.hpp"
#include "Selectable.hpp"

#include <unordered_map>
using std::unordered_map;

namespace RPGNet {

	typedef std::function<Selectable* (class Server*, SOCKET, struct sockaddr_in)> CreateClientFunction;

	class Server {
		bool m_isRunning;
		unordered_map<Selectable*, int> m_clients;
		CreateClientFunction m_createClient;

		struct TCPServer {
			int port;
			char host[256];
			SOCKET sock;

			int Initialize();
			void Cleanup();
			bool IsAlive();
			Selectable* AcceptConnection(Server* server);
		} m_TCPServer;

		struct UDPServer {
			int port;
			char host[256];
			SOCKET sock;

			int Initialize();
			void Cleanup();
			bool IsAlive();
			void ReadPacket();
		} m_UDPServer;

		void SelectablesLoop();

	public:
		Server(CreateClientFunction createClient);
		~Server();

		HBUtils::Scheduler Scheduler;
		HBUtils::Config Config;

		void Run();
		void Stop();

		// Called to notify the server that a write is now available (and select should be re-called)
		void NotifyWriteAvailable();
	};

	template<typename ClientType>
	class ServerT : public Server {
	public:
		ServerT() : Server([](Server* server, SOCKET sock, struct sockaddr_in addr) {
			return new ClientType(server, sock, addr);
		}) {}
	};

};
