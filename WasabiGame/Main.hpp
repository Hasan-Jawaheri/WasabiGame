#pragma once

#include "Common.hpp"

//******************************************************************************************
//This is the main class that inherits from HasX11. This is the "container" of the game
//******************************************************************************************
class Application : public Wasabi {

public:
	Application();

	//***************************************************************************************
	//This is called once at the beginning of the application asking it to initialize
	//***************************************************************************************
	virtual WError Setup();

	//***************************************************************************************
	//This is called every frame asking the application to update
	//This occurs before rendering and rendering should NOT be done here
	//***************************************************************************************
	virtual bool Loop(float fDeltaTime);

	//***************************************************************************************
	//This is called once at the end of the program if it passes the Setup function
	//***************************************************************************************
	virtual void Cleanup();

	virtual WError Resize(unsigned int width, unsigned int height);

	float fCurGameTime;
};
