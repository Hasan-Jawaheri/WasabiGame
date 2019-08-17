#include <WinSock2.h>
#include "RTBClient/Main.hpp"

#include "WasabiGame/Main.hpp"
#include "WasabiGame/GameStates/Intro.hpp"
#include "WasabiGame/GameStates/Menu.hpp"
#include "RTBClient/GameStates/Game.hpp"
#include "RollTheBall/Maps/RTBMaps.hpp"
#include "RollTheBall/Units/RTBUnits.hpp"

#include "RollTheBall/AssetsGenerator/AssetsGenerator.hpp"

RTBClient::RTBClient(bool generateAssets, bool enableVulkanDebugging, bool enablePhysicsDebugging) : WasabiRPG() {
	SetEngineParam("appName", "RTBClient");

#ifdef _DEBUG
	m_settings.debugVulkan = enableVulkanDebugging;
	m_settings.debugPhysics = enablePhysicsDebugging;
	m_settings.fullscreen = false;
#else
	m_settings.debugVulkan = false;
	m_settings.debugPhysics = false;
	m_settings.fullscreen = false;
#endif

	m_settings.mediaFolder = "Media/RollTheBall";

	if (generateAssets) {
		if (!AssetGenerator(m_settings.mediaFolder).Generate())
			return;
	}

	Networking = new RTBNet::RTBClientNetworking();
	Networking->Initialize();
}

RTBClient::~RTBClient() {
	Networking->Destroy();
	delete Networking;
}

void RTBClient::SwitchToInitialState() {
	SetupRTBMaps(Maps);
	SetupRTBUnits(Units, false);

	PhysicsComponent->SetGravity(0, -40, 0);

#ifndef _DEBUG
	SwitchState(new Intro(this, { "Media/RollTheBall/me3.jpg" }, [this]() { return new Menu(this, [this]() { return new Game(this); }); }));
#else
	SwitchState(new Game(this));
#endif
}
