#include "RollTheBall/Networking/Networking.hpp"

#include "WasabiGame/Main.hpp"
#include "WasabiGame/GameStates/Intro.hpp"
#include "WasabiGame/GameStates/Menu.hpp"
#include "RollTheBall/GameStates/Game.hpp"
#include "RollTheBall/Maps/RTBMaps.hpp"
#include "RollTheBall/Units/RTBUnits.hpp"

#include "RollTheBall/AssetsGenerator/AssetsGenerator.hpp"

#include <thread>

#define GENERATE_ASSETS true

RTBNet::RTBClientNetworking* gRTBNetworking;

class RTB : public WasabiRPG {

public:
	RTB() : WasabiRPG() {
		m_settings.debugVulkan = true;
		m_settings.debugPhysics = true;
		m_settings.fullscreen = false;
		m_settings.mediaFolder = "Media/RollTheBall";

		if (GENERATE_ASSETS) {
			if (!AssetGenerator(m_settings.mediaFolder).Generate())
				return;
		}

		gRTBNetworking = new RTBNet::RTBClientNetworking();
		gRTBNetworking->Initialize();
	}

	~RTB() {
		gRTBNetworking->Destroy();
		delete gRTBNetworking;
	}

	void SwitchToInitialState() {
		SetupRTBMaps();
		SetupRTBUnits();

		PhysicsComponent->SetGravity(0, -40, 0);

#ifndef _DEBUG
		SwitchState(new Intro(this, { "Media/RollTheBall/me3.jpg" }, [this]() { return new Menu(this, [this]() { return new Game(this); }); }));
#else
		SwitchState(new Game(this));
#endif
	}
};

Wasabi* WInitialize() {
	APPHANDLE = new RTB();
	return APPHANDLE;
}
