#pragma once

#include "Common.hpp"
#include "WasabiGame/GameStates/BaseState.hpp"
#include "WasabiGame/UI/UI.hpp"

class Game;

class GameInputHandler : public UIElement {
	class Game* m_game;

public:
	GameInputHandler(class Game* g);
	virtual void OnMouseButton(int mx, int my, bool bDown);
	virtual void OnMouseButton2(int mx, int my, bool bDown);
	virtual bool OnFocus() { return false; }
	virtual bool OnInput() { return true; }
	virtual bool OnEnter();
	virtual void OnKeydown(short key);
	virtual void OnKeyup(short key);
	virtual bool OnEscape();
	virtual int GetPosZ() const { return 0; }
};

class Game : public BaseState {

	friend class GameInputHandler;

	class Player* m_player;

	GameInputHandler* m_input;

public:
	Game(Wasabi* app);
	~Game();

	void Load();
	void Update(float fDeltaTime);
	void Cleanup();
};
