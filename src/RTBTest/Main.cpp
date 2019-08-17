#include <WinSock2.h>
#include "RTBClient/Main.hpp"
#include "RTBServer/Main.hpp"
#include "RollTheBall/AssetsGenerator/AssetsGenerator.hpp"

#include <thread>

#define GENERATE_ASSETS true
#define NUM_CLIENTS 2

Wasabi* WInitialize() {
	if (GENERATE_ASSETS) {
		AssetGenerator AG = AssetGenerator("Media/RollTheBall");
		if (!AG.Generate())
			return nullptr;
	}

	std::thread serverThread([]() {
		std::srand(std::time(nullptr) + 3179);
		RunRTBServer(false);
	});

	std::vector<std::thread> clientThreads;
	for (int i = 0; i < NUM_CLIENTS; i++) {
		clientThreads.push_back(std::thread([i]() {
			std::srand(std::time(nullptr) + i * 1789);
			Wasabi* client = new RTB(false, false, true);
			RunWasabi(client);
			delete client;
		}));
	}

	serverThread.join();
	for (auto it = clientThreads.begin(); it != clientThreads.end(); it++)
		(*it).join();

	return nullptr;
}
