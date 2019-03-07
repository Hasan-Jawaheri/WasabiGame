#pragma once

#include "..//Common.hpp"
#include "..//UI//UI.hpp"

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

class Game : public WGameState {

	friend class GameInputHandler;

	class Player* m_player;
	class Unit* m_target;

	GameInputHandler* m_input;
	struct USER_INTERFACE {
		class CastBar* castBar;
		class HealthBar* playerHealth;
		class HealthBar* targetHealth;
		class StanceBar* stanceBar;
		class TargetCursor* targetCursor;
		class ChatEditBox* chatEdit;
	} m_ui;

public:
	Game(Wasabi* app);
	~Game();

	void Load();
	void Update(float fDeltaTime);
	void Cleanup();
};
