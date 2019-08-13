#include <WinSock2.h>
#include "RTBClient/Main.hpp"
#include "RTBServer/Main.hpp"

#include <thread>

#define NUM_CLIENTS 1

Wasabi* WInitialize() {
	std::thread serverThread([]() {
		RunRTBServer();
	});

	std::vector<std::thread> clientThreads;
	for (int i = 0; i < NUM_CLIENTS; i++) {
		clientThreads.push_back(std::thread([]() {
			Wasabi* client = new RTB();
			RunWasabi(client);
			delete client;
		}));
	}

	serverThread.join();
	for (auto it = clientThreads.begin(); it != clientThreads.end(); it++)
		(*it).join();

	return nullptr;
}
