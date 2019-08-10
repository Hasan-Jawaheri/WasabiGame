#pragma once

#include "Common.hpp"

class Menu : public WGameState {
	std::function<WGameState* ()> m_nextStateGenerator;

public:
	Menu(Wasabi* app, std::function<WGameState* ()> nextStateGenerator);
	~Menu();

	void Load();
	void Update(float fDeltaTime);
	void Cleanup();
};
