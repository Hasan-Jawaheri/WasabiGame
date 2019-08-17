#include "Common.hpp"
#include "RTBServer/Main.hpp"

void RedirectIOToConsole() {
#ifdef _WIN32
	// allocate a console for this app
	AllocConsole();

	FILE* scout, * scin;
	freopen_s(&scout, "CONOUT$", "w", stdout);
	freopen_s(&scin, "CONIN$", "r", stdin);
#endif
}

Wasabi* WInitialize() {
	RedirectIOToConsole();

	RTBServer server;
	server.Initialize();
	server.Run();
	server.Destroy();

	return nullptr;
}