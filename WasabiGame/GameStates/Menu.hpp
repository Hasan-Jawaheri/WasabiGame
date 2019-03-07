#pragma once

#include "..//Common.hpp"

class Menu : public WGameState {
public:
	Menu(Wasabi* app);
	~Menu();

	void Load();
	void Update(float fDeltaTime);
	void Cleanup();
};
