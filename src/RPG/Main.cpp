#include "../WasabiGame/Main.hpp"
#include "../WasabiGame/GameStates/Intro.hpp"
#include "../WasabiGame/GameStates/Menu.hpp"
#include "Maps/RPGMaps.hpp"
#include "Units/RPGUnits.hpp"
#include "GameStates/Game.hpp"

class RPG : public WasabiRPG {
public:
	RPG() : WasabiRPG() {

	}

	void SwitchToInitialState() {
		SetupRPGMaps();
		SetupRPGUnits();

#ifndef _DEBUG
		SwitchState(new Intro(this, { "Media/RPG/me3.jpg" }, [this]() { return new Menu(this, [this]() { return new Game(this); }); }));
#else
		SwitchState(new Game(this));
#endif
	}
};

Wasabi* WInitialize() {
	APPHANDLE = new RPG();
	return APPHANDLE;
}
