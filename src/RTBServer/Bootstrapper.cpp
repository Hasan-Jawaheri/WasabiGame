#include "Common.hpp"
#include "RTBServer/Main.hpp"


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

	std::shared_ptr<RTBServer::ServerApplication> server = std::make_shared<RTBServer::ServerApplication>();
	server->Initialize();
	server->Run();
	server->Destroy();

	return nullptr;
}