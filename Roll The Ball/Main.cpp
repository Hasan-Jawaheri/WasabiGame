#include "../RTB Server/Lib/Server.hpp"
#include "../RTB Server/Lib/Client.hpp"

#include "../WasabiGame/Main.hpp"
#include "../WasabiGame/GameStates/Intro.hpp"
#include "../WasabiGame/GameStates/Menu.hpp"
#include "GameStates/Game.hpp"
#include "Maps/RTBMaps.hpp"
#include "Units/RTBUnits.hpp"

#include "AssetsGenerator/AssetsGenerator.hpp"

#include <thread>

#define GENERATE_ASSETS true

class RTB : public WasabiRPG {
public:
	RTB() : WasabiRPG() {
		m_settings.debugVulkan = true;
		m_settings.debugPhysics = true;
		m_settings.fullscreen = false;

		if (GENERATE_ASSETS) {
			if (!AssetGenerator().Generate())
				return;
		}
	}

	void SwitchToInitialState() {
		SetupRTBMaps();
		SetupRTBUnits();

		PhysicsComponent->SetGravity(0, -40, 0);

#ifndef _DEBUG
		SwitchState(new Intro(this, { "Media/me3.jpg" }, [this]() { return new Menu(this, [this]() { return new Game(this); }); }));
#else
		SwitchState(new Game(this));
#endif
	}
};

void NetworkingThread() {
	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);

	RPGNet::ServerT<RPGNet::Client> server;
	server.Config.Set<int>("tcpPort", 0);
	server.Config.Set<int>("udpPort", 0);
	server.Config.Set<int>("numWorkers", 1);

	RPGNet::Client tcpConnection(&server);
	RPGNet::Client udpConnection(&server);

	server.Run();
}

Wasabi* WInitialize() {
	std::thread networkingThread(NetworkingThread);
	networkingThread.detach();

	APPHANDLE = new RTB();
	return APPHANDLE;
}
