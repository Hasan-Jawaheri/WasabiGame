#include "RTBClient/Main.hpp"
#include "RTBServer/Main.hpp"
#include "RollTheBall/AssetsGenerator/AssetsGenerator.hpp"

#include <thread>

#define GENERATE_ASSETS true
#define NUM_CLIENTS 2

void RedirectIOToConsole() {
#ifdef _WIN32
	// allocate a console for this app
	AllocConsole();

	FILE *scout, *scin, *scerr;
	freopen_s(&scout, "CONOUT$", "w", stdout);
	freopen_s(&scerr, "CONOUT$", "w", stderr);
	freopen_s(&scin, "CONIN$", "r", stdin);

	// redirect stderr to the new IO console
	HANDLE stdHandle = GetStdHandle(STD_ERROR_HANDLE);
	if (stdHandle != INVALID_HANDLE_VALUE) {
		int fileDescriptor = _open_osfhandle((intptr_t)stdHandle, _O_TEXT);
		if (fileDescriptor != -1) {
			FILE* file = _fdopen(fileDescriptor, "w");
			if (file != NULL) {
				int dup2Result = _dup2(_fileno(file), _fileno(stderr));
				if (dup2Result == 0) {
					setvbuf(stderr, NULL, _IONBF, 0);
				}
			}
		}
	}
#endif

	loguru::set_thread_name("Main");
}

Wasabi* WInitialize() {
	RedirectIOToConsole();

	if (GENERATE_ASSETS) {
		RollTheBall::AssetGenerator AG = RollTheBall::AssetGenerator("Media/RollTheBall");
		if (!AG.Generate())
			return nullptr;
	}

	std::thread serverThread([]() {
		std::srand(std::time(nullptr) + 3179);
		std::shared_ptr<RTBServer::ServerApplication> server = std::make_shared<RTBServer::ServerApplication>(false);
		server.get()->Run();
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
