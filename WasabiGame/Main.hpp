#pragma once

#include "Common.hpp"

//******************************************************************************************
//This is the main class that inherits from HasX11. This is the "container" of the game
//******************************************************************************************
class Application : public Wasabi {

public:
	Application(); //constructor

	//***************************************************************************************
	//This is called once at the beginning of the application asking it to initialize
	//***************************************************************************************
	WError Setup();

	//***************************************************************************************
	//This is called every frame asking the application to update
	//This occurs before rendering and rendering should NOT be done here
	//***************************************************************************************
	bool Loop(float fDeltaTime);

	//***************************************************************************************
	//This is called once at the end of the program if it passes the Setup function
	//***************************************************************************************
	void Cleanup();

	//***************************************************************************************
	//This is called twice, once with bBeforceProcessing set to true and then false. This
	//happens everytime windows sends a message to the window.
	//***************************************************************************************
	int WindowCallback(HWND wnd, UINT msg, WPARAM w, LPARAM l, bool bBeforeProcessing);

	float fCurGameTime;
};
