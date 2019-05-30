#include "../WasabiGame/Main.hpp"
#include "../WasabiGame/GameStates/Intro.hpp"
#include "../WasabiGame/GameStates/Menu.hpp"
#include "GameStates/Game.hpp"

class RPG : public WasabiRPG {
public:
	RPG() : WasabiRPG() {

	}

	void SwitchToInitialState() {
#ifndef _DEBUG
		SwitchState(new Intro(this, { "Media/me3.jpg" }, []() { return new Menu(this) }));
#else
		SwitchState(new Game(this));
#endif
	}
};

Wasabi* WInitialize() {
	APPHANDLE = new RPG();
	return APPHANDLE;
}
