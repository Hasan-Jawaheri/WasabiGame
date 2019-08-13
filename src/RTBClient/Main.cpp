#include <WinSock2.h>
#include "RTBClient/Main.hpp"

#include "WasabiGame/Main.hpp"
#include "WasabiGame/GameStates/Intro.hpp"
#include "WasabiGame/GameStates/Menu.hpp"
#include "RTBClient/GameStates/Game.hpp"
#include "RollTheBall/Maps/RTBMaps.hpp"
#include "RollTheBall/Units/RTBUnits.hpp"

#include "RollTheBall/AssetsGenerator/AssetsGenerator.hpp"

#define GENERATE_ASSETS true

RTB::RTB() : WasabiRPG() {
	m_settings.debugVulkan = true;
	m_settings.debugPhysics = true;
	m_settings.fullscreen = false;
	m_settings.mediaFolder = "Media/RollTheBall";

	if (GENERATE_ASSETS) {
		if (!AssetGenerator(m_settings.mediaFolder).Generate())
			return;
	}

	RTBNetworking = new RTBNet::RTBClientNetworking();
	RTBNetworking->Initialize();

	RTBNetworking->RegisterNetworkUpdateCallback(RTBNet::UPDATE_TYPE_UNIT, [this](RPGNet::NetworkUpdate& update) {
		uint32_t unitId = update.targetId;
		Unit* unit = Units->GetUnit(unitId);
	});
}

RTB::~RTB() {
	RTBNetworking->Destroy();
	delete RTBNetworking;
}

void RTB::SwitchToInitialState() {
	SetupRTBMaps(Maps);
	SetupRTBUnits(Units);

	PhysicsComponent->SetGravity(0, -40, 0);

#ifndef _DEBUG
	SwitchState(new Intro(this, { "Media/RollTheBall/me3.jpg" }, [this]() { return new Menu(this, [this]() { return new Game(this); }); }));
#else
	SwitchState(new Game(this));
#endif
}
