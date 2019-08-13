#include <WinSock2.h>
#include <Wasabi/Wasabi.h>
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
	RunRTBServer();
	return nullptr;
}