#include "RTBClient/Main.hpp"
#include "RTBServer/Main.hpp"
#include "RollTheBall/AssetsGenerator/AssetsGenerator.hpp"

#include <thread>

#define GENERATE_ASSETS true
#define NUM_CLIENTS 2

Wasabi* WInitialize() {
	if (GENERATE_ASSETS) {
		RollTheBall::AssetGenerator AG = RollTheBall::AssetGenerator("Media/RollTheBall");
		if (!AG.Generate())
			return nullptr;
	}

	std::thread serverThread([]() {
		std::srand(std::time(nullptr) + 3179);
		std::shared_ptr<RTBServer::ServerApplication> server = std::make_shared<RTBServer::ServerApplication>();
		server->Initialize(false);
		server->Run();
		server->Destroy();
	});

	std::vector<std::thread> clientThreads;
	for (int i = 0; i < NUM_CLIENTS; i++) {
		clientThreads.push_back(std::thread([i]() {
			std::srand(std::time(nullptr) + i * 1789);
			std::shared_ptr<RTBClient::ClientApplication> client = std::make_shared<RTBClient::ClientApplication>(false, false, false);
			RunWasabi(client.get());
		}));
	}

	serverThread.join();
	for (auto it = clientThreads.begin(); it != clientThreads.end(); it++)
		(*it).join();

	return nullptr;
}
