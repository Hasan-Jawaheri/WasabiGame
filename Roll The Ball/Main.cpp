#include "../WasabiGame/Main.hpp"
#include "../WasabiGame/GameStates/Intro.hpp"
#include "../WasabiGame/GameStates/Menu.hpp"
#include "GameStates/Game.hpp"
#include "Maps/RTBMaps.hpp"
#include "Units/RTBUnits.hpp"

#include "AssetsGenerator/AssetsGenerator.hpp"

#define GENERATE_ASSETS true

class RTB : public WasabiRPG {
public:
	RTB() : WasabiRPG() {
		m_settings.debugVulkan = true;
		m_settings.debugPhysics = true;

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
		SwitchState(new Intro(this, { "Media/me3.jpg" }, []() { return new Menu(this) }));
#else
		SwitchState(new Game(this));
#endif
	}
};

Wasabi* WInitialize() {
	APPHANDLE = new RTB();
	return APPHANDLE;
}
