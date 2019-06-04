#include <WinSock2.h>
#include <Wasabi.h>
#include "Server.hpp"
#include "Clients/Client.hpp"

/*
#include <Wasabi.h>
#include <Physics/Bullet/WBulletPhysics.h>

class PhysicsOnlyWasabi : public Wasabi {
public:
	WError Setup() {
		PhysicsComponent = new WBulletPhysics(this);
		return WError();
	}
	bool Loop(float fDeltaTime) { return true; }
	void Cleanup() {}
};
*/

void RedirectIOToConsole() {
#ifdef _WIN32
	// allocate a console for this app
	AllocConsole();

	FILE *scout, *scin;
	freopen_s(&scout, "CONOUT$", "w", stdout);
	freopen_s(&scin, "CONIN$", "r", stdin);
#endif
}

Wasabi* WInitialize() {
	RedirectIOToConsole();

	RPGNet::ServerT<RPGNet::Client> server;
	server.Run();

	RunWasabi(nullptr); // this forces the linker to find WinMain in WCore.cpp in Wasabi.lib...
	return nullptr;
}